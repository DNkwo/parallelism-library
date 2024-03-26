#ifndef PIPE_HPP
#define PIPE_HPP

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
        ThreadSafeQueue<Task> currentInputQueue = inputQueue;
        ThreadSafeQueue<Result> outputQueue;
        
        for (auto& stage : stages) {
            outputQueue = stage->process(currentInputQueue);

            //add outputQueue into new inputQueue
            ThreadSafeQueue<Task> newInputQueue;

            //collection of results
            while(!outputQueue.empty()) {
                Result result;
                while(outputQueue.dequeue(result)) {
                    Task newTask(result.data); //conversion of result to a task
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