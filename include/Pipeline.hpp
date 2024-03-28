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
        if (!stages.empty()) {
            //set all the workers of the current stage's output queue to point to the input queue of the succeeding stage
            std::vector<Worker*> workers = stages.back()->getWorkers();
            for(Worker* worker : workers) {
                worker->outputQueue = stage->getInputQueue();
            }
        }

        stages.push_back(stage);
        //connecting the outqueue's of a previous stage to the single aggregate input queue of the succeeding stage
    }

    ThreadSafeQueue<Result> execute(ThreadSafeQueue<Task>& inputQueue) {
        
        //enqueue all tasks from input queue to first stage's input queue
        Task task;
        while (inputQueue.dequeue(task)) {
            stages[0]->getInputQueue()->enqueue(task);
        }


        //also add eosTask
        Task eosTask(nullptr, true);
        stages[0]->getInputQueue()->enqueue(eosTask);

        // continuously check output queue of final stage for results
        ThreadSafeQueue<Result> results;
        while (true) {
            std::vector<Worker*> workers = stages.back()->getWorkers();
            for(Worker* worker : workers) {
                while(!worker->outputQueue->empty()) {
                    Task task;
                    if (worker->outputQueue->dequeue(task)) {
                        if (task.isEOS) {
                            return results;
                        }
                        Result result;
                        result.data = task.data;
                        results.enqueue(result);
                    }
                }
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
        Task shutdownTask;
        shutdownTask.isShutdown = true;
        stages[0]->getInputQueue()->enqueue(shutdownTask);
    }
};

#endif