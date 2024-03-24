#include "include/Farm.hpp"
#include <iostream>
#include <unistd.h>
#include <queue>

Farm::Farm(int numOfWorkers, WorkerFunction workerFunction)
    : numOfWorkers(numOfWorkers), workerFunction(workerFunction) {
    workers.resize(numOfWorkers);
    //creatte worker threads
    for (int i = 0; i < numOfWorkers; ++i) {
        pthread_t thread;
        pthread_create(&thread, nullptr, workerFunction, &workers[i]); //passes the worker[i] as an argument to the workerFunction
        threads.push_back(thread);
    }
}

Farm::~Farm() {
    stop = true;
    for (auto& t : threads) {
        if (t) {
            pthread_join(t, nullptr); //ensure all threads are joined
        }
    }
}

void Farm::submitTask(int workerId, Task task) {
    //add task to specific worker's input queue
    pthread_mutex_lock(&workers[workerId].inputMutex);
    workers[workerId].inputQueue.push(task);
    pthread_mutex_unlock(&workers[workerId].inputMutex);
}

// Add a task to the output queue for a specific worker
void Farm::enqueueResult(int workerId, const Result& result) {
    pthread_mutex_lock(&workers[workerId].outputMutex);
    std::cout << "result has been pushed" << std::endl;
    workers[workerId].outputQueue.push(result);
    pthread_mutex_unlock(&workers[workerId].outputMutex);
}

// Remove and retrieve a task from the output queue for a specific worker
bool Farm::dequeueResult(int workerId, Result& result) {
    pthread_mutex_lock(&workers[workerId].outputMutex);
    if (!workers[workerId].outputQueue.empty()) {
        result = workers[workerId].outputQueue.front();
        workers[workerId].outputQueue.pop();
        pthread_mutex_unlock(&workers[workerId].outputMutex);
        return true;
    } else {
        pthread_mutex_unlock(&workers[workerId].outputMutex);
        return false; //queue is empty and thereforre no result has been dequeued
    }

}

//Instead of making an EOS flag directly to the worker, creating an EOS task instead allows tasks to finish
//without abruptly shutting down and extra synchronisation
void Farm::signalEOS() {
    //EOS task
    Task eosTask;
    eosTask.isEOS = true;
    for(int i = 0; i < numOfWorkers; ++i) {
        submitTask(i, eosTask);
    }
}

//method to manually stop all workers (handles thread clean up)
void Farm::stopWorkers() {

    
    //signal all worker threads to stop manually
    // for (Worker& worker : workers) {
    //     worker.stopRequested = true;
    // }

    //join all worker threads to ensure they've finished
    for (auto& thread : threads) {
        pthread_join(thread, nullptr);
    }
}