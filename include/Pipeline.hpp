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

            // Task result;

            // //add outputQueue into new inputQueue
            // ThreadSafeQueue<Task> newInputQueue;

            // //passing the results down to the next stage
            // while(!outputQueue.empty()) {
            //     Task result;
            //     if(outputQueue.dequeue(result)) {
            //         Task* newTask = new Task(result.data); //conversion of result to a task
            //         newInputQueue.enqueue(newTask);
            //     }
            // }

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
};

#endif