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

    ThreadSafeQueue<Result> execute(ThreadSafeQueue<Task>& inputQueue) {
        //round-robin the tasks evenly in the first stage
        std::vector<Worker*> firstStageWorkers = stages[0]->getWorkers();
        size_t currentWorker = 0;
        while (!inputQueue.empty()) {
            Task task;
            if (inputQueue.dequeue(task)) {
                firstStageWorkers[currentWorker]->inputQueue->enqueue(task);
                currentWorker = (currentWorker + 1) % firstStageWorkers.size();
            }
        }

        // also enqueue eosTask tasks for all workers in the first stage
        for (Worker* worker : firstStageWorkers) {
            worker->inputQueue->enqueue(Task(nullptr, true));
        }

        size_t totalWorkers = 0;
        for (const auto& stage : stages) {
            totalWorkers += stage->getWorkers().size();
        }

        ThreadSafeQueue<Result> output;
        std::vector<Worker*> finalStageWorkers = stages.back()->getWorkers();
        //we wait for the EOS task to be processed by all workers in the final stage
        size_t completedWorkers = 0;
        while (completedWorkers < totalWorkers) { //only when all workers have received an EOS, can we leave the loop
            for (Worker* worker : finalStageWorkers) {
                Task task;
                if (worker->outputQueue->dequeue(task)) {
                    if (task.isEOS) {
                        completedWorkers++;
                        continue;
                    } else if (!task.isShutdown) { //if this task ins't an eos task or shutdown task
                        Result result;
                        result.data = task.data;
                        output.enqueue(result);
                    }
                }
            }
            sched_yield();
        }


        return output;
    }

    //destructor (for clean up)
    ~Pipeline() {
        for (auto* stage : stages) {
            std::vector<Worker*> stageWorkers = stage->getWorkers();
            for (Worker* worker : stageWorkers) {
                delete worker;
            }
        delete stage; //free each stage 
    }
    }

    void terminate() {
        //enqueue shutdown tasks for all workers in the first stage
        std::vector<Worker*> firstStageWorkers = stages[0]->getWorkers();
        for (Worker* worker : firstStageWorkers) {
            Task shutdownTask;
            shutdownTask.isShutdown = true;
            worker->inputQueue->enqueue(shutdownTask);
        }

        //we ensure that all worker threads have processed all tasked 
        //if we don't wait for worker threads to finish, then tasks may still be
        //in queues when manager.stop(), preventing further propagation
        //and therefore the threads will never shutdown
        for (Stage<Task>* stage : stages) {
            std::vector<Worker*> stageWorkers = stage->getWorkers();
            for (Worker* worker : stageWorkers) {
                pthread_join(worker->thread, nullptr);
            }
        }
    }

    std::vector<Stage<Task>*> getStages() {
        return stages;
    }

};

#endif