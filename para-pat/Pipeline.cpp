#include "include/Pipeline.hpp"
#include <chrono>
#include <Windows.h>

void Pipeline::addStage(Stage<Task>* stage) {
    stages.push_back(stage);
}


ThreadSafeQueue<Result> Pipeline::execute(ThreadSafeQueue<Task>& inputQueue) {
    size_t totalTasks = inputQueue.size();
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

    
    ThreadSafeQueue<Result> output;
    std::vector<Worker*> finalStageWorkers = stages.back()->getWorkers();
    size_t totalWorkers = finalStageWorkers.size();


    //we wait for the EOS task to be processed by all workers in the final stage
    size_t resultCount = 0;
    while (resultCount < totalTasks) { //only when all workers have received an EOS, can we leave the loop
        for (Worker* worker : finalStageWorkers) {
            Task task;
            while (worker->outputQueue->dequeue(task)) {
                if (task.isEOS) {
                    continue;
                } else if (!task.isShutdown) { //if this task ins't an eos task or shutdown task
                    Result result;
                    result.data = task.data;
                    output.enqueue(result);
                    resultCount++;
                }
            }
        }
        sched_yield();
    }

    return output;
}

Pipeline::~Pipeline() {
    // for (auto* stage : stages) {
    //     std::vector<Worker*> stageWorkers = stage->getWorkers();
    //     for (Worker* worker : stageWorkers) {
    //         delete worker;
    //     }
    //     delete stage; //free each stage 
    // }
}

void Pipeline::test() {
    for (auto* stage : stages) {
        std::vector<Worker*> stageWorkers = stage->getWorkers();
        for (Worker* worker : stageWorkers) {
            if(worker->outputQueue->size() > 0) {
                std::cout << "whaat" << std::endl;
            }
            delete worker;
        }
        // delete stage; //free each stage 
    }
}

void Pipeline::terminate() {
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

std::vector<Stage<Task>*> Pipeline::getStages() {
    return stages;
}