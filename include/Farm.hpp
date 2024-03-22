#ifndef FARM_HPP
#define FARM_HPP


#include "ParallelPattern.hpp"

#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <queue>

//define worker function type
typedef void* (*WorkerFunction)(void*);

struct Task {
    int value = 0;
    bool isValid = false; //by default
    bool isEOS = false;
};

struct Result {
    int value;
};

class Worker {
public:
    std::queue<Task> inputQueue;
    pthread_mutex_t inputMutex;

    std::queue<Result> outputQueue;
    pthread_mutex_t outputMutex;

    // we use this flag to singal worker to stop (the thread, regardless of state of task queue)
    volatile bool stopRequested; // (volatile ensures threads do not miss updates from main thread)

    Worker() : stopRequested(false) {
        //initialise the mutexes
        pthread_mutex_init(&inputMutex, nullptr);
        pthread_mutex_init(&outputMutex, nullptr);
    }

    ~Worker() {
        //destroy mutexes
        pthread_mutex_destroy(&inputMutex);
        pthread_mutex_destroy(&outputMutex);
    }
};

class Farm : public ParallelPattern {
private:
    size_t numOfWorkers;
    std::vector<pthread_t> threads;
    std::vector<Worker> workers;
    WorkerFunction workerFunction;
    bool stop;

public:
    Farm(int numOfWorkers, WorkerFunction workerFunction);

    ~Farm(); //destructor

    void submitTask(int workerId, Task task);

    //adding and removing tasks from outqueue, using these methods ensure thread-safe ways of modifying the outputqueue
    void enqueueResult(int workerId, const Result& result);

    bool dequeueResult(int workerId, Result& result);

    void signalEOS();

    void stopWorkers();
};

#endif