#ifndef STAGE_MANAGER_HPP
#define STAGE_MANAGER_HPP

#include <utility>
#include <vector>
#include "Stage.hpp"
#include "Pipeline.hpp"

class StageManager {
private:
    Pipeline pipeline;
    std::vector<std::pair<Stage<Task>*, Stage<Task>*>> interrelations;
    volatile bool stopRequested = false;
    pthread_t managerThread;

    //input queue
    // ThreadSafeQueue<Task> inputQueue;

public:

    StageManager() {}

    void addStage(Stage<Task>* stage);

    // void addTask(Task& task);

    void setupInterrelations();

    ThreadSafeQueue<Result> execute(ThreadSafeQueue<Task>& inputQueue);

    void terminate();

    static void* managerThreadWrapper(void* arg);

    void run();

};

#endif