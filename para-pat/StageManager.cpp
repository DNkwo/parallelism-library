#include "include/StageManager.hpp"

void StageManager::addStage(Stage<Task>* stage) {
    pipeline.addStage(stage);
}

// void StageManager::addTask(Task& task) {
//     inputQueue.enqueue(task);
// }

void StageManager::setupInterrelations() {
    std::vector<Stage<Task>*> stages = pipeline.getStages();
    for (size_t i = 0; i < stages.size() - 1; ++i) {
        //avoid having to use make_pair, constructs the object directly
        interrelations.emplace_back(stages[i], stages[i + 1]);
    }
}

ThreadSafeQueue<Result> StageManager::execute(ThreadSafeQueue<Task>& inputQueue) {
    setupInterrelations();
    pthread_create(&managerThread, nullptr, managerThreadWrapper, this);

    return pipeline.execute(inputQueue);
}

void StageManager::terminate() {
    pipeline.terminate();
    stopRequested = true;
    pthread_join(managerThread, nullptr);
}

void* StageManager::managerThreadWrapper(void* arg) {
    StageManager* manager = static_cast<StageManager*>(arg);
    manager->run();
    return nullptr;
}

void StageManager::run() {
    while (!stopRequested) {
        for (const auto& interrelation : interrelations) {
            Stage<Task>* stage1 = interrelation.first;
            Stage<Task>* stage2 = interrelation.second;

            std::vector<Worker*> stage1Workers = stage1->getWorkers();
            std::vector<Worker*> stage2Workers = stage2->getWorkers();

            //we use a round robin approach for task distribution
            size_t currentWorker = 0;
            for (Worker* worker : stage1Workers) { //loop through stage 1 workers
                Task task;
                while (worker->outputQueue->dequeue(task)) { // dequeues from stage1 output queue
                    //enqueue EOS task to all workers in stage 2
                    if (task.isEOS || task.isShutdown) {
                        for (Worker* stage2Worker : stage2Workers) {
                            stage2Worker->inputQueue->enqueue(task);
                        }
                        break;
                    } else {
                        stage2Workers[currentWorker]->inputQueue->enqueue(task); //place in a stage 2 worker input queue
                        currentWorker = (currentWorker + 1) % stage2Workers.size(); //update index (max is total workers in stage 2)
                    }
                }
            }
        }
        sched_yield();
    }
}