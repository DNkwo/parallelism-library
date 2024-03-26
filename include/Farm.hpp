#ifndef FARM_HPP
#define FARM_HPP

#include "Stage.hpp"
#include "ThreadSafeQueue.hpp"

#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>

#define QUEUE_SIZE 100

class Worker {
public:
    size_t id = 0;
    //thread-safety implementation of queues (using pthread mutex)
    ThreadSafeQueue<Task> inputQueue; 
    ThreadSafeQueue<Result> outputQueue;

    WorkerFunction workerFunction = nullptr;

    // we use this flag to singal worker to stop (the thread, regardless of state of task queue)
    volatile bool stopRequested = false; // (volatile ensures threads do not miss updates from main thread)

    Worker() : stopRequested(false) {}

    ~Worker() {}
};

class Farm : public Stage<Task, Result> {
private:
    size_t numOfWorkers;
    std::vector<pthread_t> threads;
    std::vector<Worker> workers;

    size_t currentWorker = 0; //current target worker (used for round-robin distribution)

public:
    Farm(int numOfWorkers, WorkerFunction workerFunction);

    ~Farm(); //destructor

    ThreadSafeQueue<Result> process(ThreadSafeQueue<Task>& inputQueue) override;

    void distributeTasks(ThreadSafeQueue<Task>& tasks);

    //adding and removing tasks from outqueue, using these methods ensure thread-safe ways of modifying the outputqueue
    void enqueueResult(int workerId, const Result& result);

    bool dequeueResult(int workerId, Result& result);

    void joinThreads();

    void signalEOS();

    void stopWorkers();
};

#endif