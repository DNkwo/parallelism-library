#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "Stage.hpp"
#include <vector>
#include <queue>

typedef void* (*WorkerFunction)(void*);

class Pipeline {
public:
    std::vector<Stage<Task>*> stages; //vector of pointers to the stages

    void addStage(Stage<Task>* stage) {
        stages.push_back(stage);
    }

    //execute pipeline on input queue (of tasks)
    ThreadSafeQueue<Result> execute(ThreadSafeQueue<Task>& inputQueue) {
        ThreadSafeQueue<Task> currentInputQueue = inputQueue; //creates a copy of inputQueue
        ThreadSafeQueue<Task> outputQueue;

        //append Eos-Task to indicate end of stream (batch)
        Task eosTask(nullptr, true);
        currentInputQueue.enqueue(eosTask);

        int numOfStages = stages.size();
        
        for (int i = 0; i < numOfStages; ++i) {
            outputQueue = stages[i]->process(currentInputQueue);

            //if at last stage, we break off loop, no need to prepare for next stage
            if(i == numOfStages-1) {
                break;
            }

            //pass to next pipe
            currentInputQueue = outputQueue;
        }

        ThreadSafeQueue<Result> results;
        //converting into Result
        while(!outputQueue.empty()) {
            Task task;
            if(outputQueue.dequeue(task) && !task.isEOS) { //ignores eosTasks from the result
                Result result; //conversion of task to result
                result.data = task.data;
                results.enqueue(result);
            }
        }

        return results;
    }

    //destructor (for clean up)
    ~Pipeline() {
        for (auto* stage : stages) {
            delete stage; //free each stage 
        }
    }


    void terminate() {
        ThreadSafeQueue<Task> currentInputQueue; //creates a copy of inputQueue
        ThreadSafeQueue<Task> outputQueue;

        Task shutdownTask;
        shutdownTask.isShutdown = true;
        currentInputQueue.enqueue(shutdownTask);

        int numOfStages = stages.size();

        for (int i = 0; i < numOfStages; ++i) {
            outputQueue = stages[i]->process(currentInputQueue);

            //if at last stage, we break off loop, no need to prepare for next stage
            if(i == numOfStages-1) {
                break;
            }

            //pass to next pipe
            currentInputQueue = outputQueue;
        }
    }
};

#endif