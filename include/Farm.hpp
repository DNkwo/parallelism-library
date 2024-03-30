#ifndef FARM_HPP
#define FARM_HPP

#include "Stage.hpp"
#include "ThreadSafeQueue.hpp"

#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>

#define QUEUE_SIZE 100


class Farm : public Stage<Task> {
private:
    size_t numOfWorkers;
    size_t currentWorker = 0; //current target worker (used for round-robin distribution)

public:
    Farm(int numOfWorkers, WorkerFunction workerFunction);

    ~Farm(); //destructor

    ThreadSafeQueue<Task> process(ThreadSafeQueue<Task>& inputQueue) override;

    void distributeTasks(ThreadSafeQueue<Task>& tasks);

    //adding and removing tasks from outqueue, using these methods ensure thread-safe ways of modifying the outputqueue
    void enqueueResult(int workerId, const Task& result);

    bool dequeueResult(int workerId, Task& result);

    void joinThreads();

    void signalEOS() override;

    void stopWorkers();

};

#endif