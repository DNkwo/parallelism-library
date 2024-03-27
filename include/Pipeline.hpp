#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "Stage.hpp"
#include <vector>
#include <queue>

typedef void* (*WorkerFunction)(void*);

class Pipeline {
public:
    std::vector<Stage<Task, Result>*> stages; //vector of pointers to the stages

    void addStage(Stage<Task, Result>* stage) {
        stages.push_back(stage);
    }

    //execute pipeline on input queue (of tasks)
    ThreadSafeQueue<Result> execute(ThreadSafeQueue<Task>& inputQueue) {
        ThreadSafeQueue<Task> currentInputQueue = inputQueue; //creates a copy of inputQueue
        ThreadSafeQueue<Result> outputQueue;

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

            Result result;

            //add outputQueue into new inputQueue
            ThreadSafeQueue<Task> newInputQueue;

            //collection of results
            while(!outputQueue.empty()) {
                Result result;
                if(outputQueue.dequeue(result)) {
                    Task* newTask = new Task(result.data); //conversion of result to a task
                    newInputQueue.enqueue(newTask);
                }
            }

            //pass to next pipe
            currentInputQueue = newInputQueue;
        }


        return outputQueue;
    }

    //destructor (for clean up)
    ~Pipeline() {
        for (auto* stage : stages) {
            delete stage; //free each stage 
        }
    }
};

#endif