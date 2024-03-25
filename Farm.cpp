#include "include/Farm.hpp"
#include <iostream>
#include <unistd.h>
#include <queue>

Farm::Farm(int numOfWorkers, WorkerFunction workerFunction)
    : numOfWorkers(numOfWorkers), workerFunction(workerFunction) {
    workers.resize(numOfWorkers);

    //assign id and the worker function to each worker
    for (int i = 0; i < numOfWorkers; ++i) {
        workers[i].id = i + 1;
        workers[i].workerFunction = workerFunction;
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

Task gettask(std::queue<Task>& inputQueue, pthread_mutex_t& inputMutex) {
    //attempt to get a task from the input queue
    pthread_mutex_lock(&inputMutex);
    Task task;
    if (!inputQueue.empty()) {
        task = inputQueue.front();
        inputQueue.pop();
        task.isValid = true; //mark as valid task
    }
    pthread_mutex_unlock(&inputMutex);
    return task; //return task that is not valid
}


void puttask(std::queue<Result>& outputQueue, pthread_mutex_t& outputMutex, const Result& result) {
    pthread_mutex_lock(&outputMutex);
    outputQueue.push(result);
    pthread_mutex_unlock(&outputMutex);
}

//The worker wrapper abstracts the queuing logic, ensuring thread-safety and allowing us to focus on processing tasks
void* workerWrapper(void* arg) {
    Worker* worker = static_cast<Worker*>(arg); //passes through a pointer to the worker instnace

    while (!worker->stopRequested) { //manual stopping method
        Task task = gettask(worker->inputQueue, worker->inputMutex);

        if (task.isEOS) {//if this is the terminating task
            break;
        }

        if(task.isValid) {
            Result result;
            void* output = worker->workerFunction(task.data);
            result.data = output;
            puttask(worker->outputQueue, worker->outputMutex, result);
            task.isValid = false; //now that task has been completed, we invalidate it,
        } else {
            sched_yield(); //yield if no task was fetched, allows to reduce busy waiting
        }

    }

    return nullptr;

}


Result Farm::addTasksAndProcess(const std::vector<Task>& tasks) {
    //preparing tasks
    distributeTasks(tasks);
    signalEOS(); //also enqueue end of stream tasks

    //create worker threads
    for (int i = 0; i < numOfWorkers; ++i) {
        pthread_t thread;
        pthread_create(&thread, nullptr, workerWrapper, &workers[i]); //passes the worker[i] as an argument to the workerFunction
        threads.push_back(thread);
    }
    joinThreads();

    Result res;

    //collection of results
    return res;
}


//uses round-robin
void Farm::distributeTasks(const std::vector<Task>& tasks) {
    size_t currentWorker = 0;

    for (const Task& task : tasks) {
        //lock mutex for current worker's queue
        pthread_mutex_lock(&workers[currentWorker].inputMutex);
        workers[currentWorker].inputQueue.push(task);
        pthread_mutex_unlock(&workers[currentWorker].inputMutex);
        //move to next worker
        currentWorker = (currentWorker + 1) % numOfWorkers;
    }
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
    std::vector<Task> eosTasks;
    //EOS task
    Task eosTask(nullptr, true);
    for(int i = 0; i < numOfWorkers; ++i) {
        eosTasks.push_back(eosTask);
    }
    distributeTasks(eosTasks);
}

void Farm::joinThreads() {
    //join all worker threads to ensure they've finished
    for (auto& thread : threads) {
        pthread_join(thread, nullptr);
    }
}

//method to manually stop all workers (handles thread clean up)
void Farm::stopWorkers() {
    //signal all worker threads to stop manually
    for (Worker& worker : workers) {
        worker.stopRequested = true;
    }
}





