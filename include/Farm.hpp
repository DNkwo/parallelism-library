#ifndef FARM_HPP
#define FARM_HPP

#include "ParaPat.hpp"

#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <queue>

//define worker function type
typedef void* (*WorkerFunction)(void*);

struct Result {
    void* data;
};

typedef Result (*TaskFunction)(void*);

struct Task {
    TaskFunction function;
    void* arg;
    bool isValid = true; //by default
    bool isEOS = false;

    //constructors
    Task() : function(nullptr), arg(nullptr), isEOS(false), isValid(false) {} //default constructor
    Task(TaskFunction func, void* argument, bool endOfStream = false) 
    : function(func), arg(argument), isValid(true), isEOS(endOfStream) {}
};


class Worker {
public:
    std::queue<Task> inputQueue;
    pthread_mutex_t inputMutex;

    std::queue<Result> outputQueue;
    pthread_mutex_t outputMutex;

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

    void processTasks(const std::vector<Task>& tasks);

    void distributeTasks(const std::vector<Task>& tasks);

    //adding and removing tasks from outqueue, using these methods ensure thread-safe ways of modifying the outputqueue
    void enqueueResult(int workerId, const Result& result);

    bool dequeueResult(int workerId, Result& result);

    void joinThreads();

    void signalEOS();

    void stopWorkers();
};

#endif