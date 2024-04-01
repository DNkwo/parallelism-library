#include "include/Pipeline.hpp"
#include <chrono>
#include <Windows.h>

void Pipeline::addStage(Stage<Task>* stage) {
    stages.push_back(stage);
}

void Pipeline::connectStages() {

    if(stages.size() > 1){
        for (size_t i = 0; i < stages.size() - 1; ++i) {
            Stage<Task>* currentStage = stages[i];
            Stage<Task>* nextStage = stages[i + 1];
            currentStage->connectWorkers(nextStage->getWorkers());
        }
    }

    //initialise queues for final stage 
    Stage<Task>* finalStage = stages.back();
    std::vector<Worker*> finalStageWorkers = finalStage->getWorkers();

    for (Worker* worker : finalStageWorkers) {
        ThreadSafeQueue<Task>* outputQueue = new ThreadSafeQueue<Task>();
        worker->outputQueues.push_back(outputQueue);
    }

}


ThreadSafeQueue<Result> Pipeline::execute(ThreadSafeQueue<Task>& inputQueue) {
    // Connect the stages
    connectStages();

    // Start the pipeline execution
    std::vector<Worker*> firstStageWorkers = stages[0]->getWorkers();
    size_t totalTasks = inputQueue.size();
    size_t currentWorker = 0;
    while (!inputQueue.empty()) {
        Task task;
        if (inputQueue.dequeue(task)) {
            firstStageWorkers[currentWorker]->inputQueue->enqueue(task);
            currentWorker = (currentWorker + 1) % firstStageWorkers.size();
        }
    }

    // Enqueue EOS tasks for all workers in the first stage
    for (Worker* worker : firstStageWorkers) {
        worker->inputQueue->enqueue(Task(nullptr, true));
    }

    // Collect the results from the last stage
    ThreadSafeQueue<Result> output;
    std::vector<Worker*> lastStageWorkers = stages.back()->getWorkers();
    size_t resultCount = 0;
    while (resultCount < totalTasks) {
        for (Worker* worker : lastStageWorkers) {
            Task task;
            for (ThreadSafeQueue<Task>* outputQueue : worker->outputQueues) {
                while (outputQueue->dequeue(task)) {
                    if (!task.isEOS && !task.isShutdown) {
                        Result result;
                        result.data = task.data;
                        output.enqueue(result);
                        resultCount++;
                    }
                }
            }
        }
        sched_yield();
    }

    return output;
}

Pipeline::~Pipeline() {
    for (auto* stage : stages) {
        std::vector<Worker*> stageWorkers = stage->getWorkers();
        for (Worker* worker : stageWorkers) {
            delete worker;
        }
        // delete stage; //free each stage 
    }
}

void Pipeline::test() {
    // for (auto* stage : stages) {
    //     std::vector<Worker*> stageWorkers = stage->getWorkers();
    //     for (Worker* worker : stageWorkers) {
    //         if(worker->outputQueue->size() > 0) {
    //             // std::cout << "whaat" << std::endl;
    //         }
    //         delete worker;
    //     }
    //     // delete stage; //free each stage 
    // }
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