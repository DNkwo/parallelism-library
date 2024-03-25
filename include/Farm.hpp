#ifndef FARM_HPP
#define FARM_HPP

#include "ParaPat.hpp"

#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <queue>


struct Result {
    void* data;
};

//define worker function type
typedef void* (*WorkerFunction)(void*);


struct Task {
    void* data = nullptr;
    bool isValid = true; //by default
    bool isEOS = false;


    //default construct
    Task() : data(nullptr), isValid(true), isEOS(false) {}

    //constructors
    Task(void* data) 
    :  data(data), isValid(true), isEOS(false) {}


    Task(void* data, bool isEOS) 
    :  data(data), isValid(true), isEOS(isEOS) {}
};


class Worker {
public:
    size_t id = 0;
    std::queue<Task> inputQueue;
    pthread_mutex_t inputMutex;

    std::queue<Result> outputQueue;
    pthread_mutex_t outputMutex;

    WorkerFunction workerFunction = nullptr;

    // we use this flag to singal worker to stop (the thread, regardless of state of task queue)
    volatile bool stopRequested = false; // (volatile ensures threads do not miss updates from main thread)

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

class Farm : public ParaPat {
private:
    size_t numOfWorkers;
    std::vector<pthread_t> threads;
    std::vector<Worker> workers;
    WorkerFunction workerFunction;
    bool stop;

public:
    Farm(int numOfWorkers, WorkerFunction workerFunction);

    ~Farm(); //destructor

    Result addTasksAndProcess(const std::vector<Task>& tasks);

    void distributeTasks(const std::vector<Task>& tasks);

    //adding and removing tasks from outqueue, using these methods ensure thread-safe ways of modifying the outputqueue
    void enqueueResult(int workerId, const Result& result);

    bool dequeueResult(int workerId, Result& result);

    void joinThreads();

    void signalEOS();

    void stopWorkers();
};

#endif