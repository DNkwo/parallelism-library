#include "include/Farm.hpp"
#include <iostream>
#include <unistd.h>
#include <queue>
#include <vector>

Farm::Farm(int numOfWorkers, WorkerFunction workerFunction)
    : Stage<Task, Result>(workerFunction), numOfWorkers(numOfWorkers) {
    workers.resize(numOfWorkers);

    //assign id and the worker function to each worker
    for (int i = 0; i < numOfWorkers; ++i) {
        workers[i].id = i + 1;
        workers[i].workerFunction = workerFunction;
    }

}

Farm::~Farm() {
    for (auto& t : threads) {
        if (t) {
            pthread_join(t, nullptr); //ensure all threads are joined
        }
    }
}

Task gettask(ThreadSafeQueue<Task>& inputQueue) {
    //attempt to get a task from the input queue
    Task task;
    task.isValid = inputQueue.dequeue(task); //stores task in 'task' and marks task as valid if successful return
    return task; //return task
}


void puttask(ThreadSafeQueue<Result>& outputQueue, const Result& result) {
    outputQueue.enqueue(result);
}

//The worker wrapper abstracts the queuing logic, ensuring thread-safety and allowing us to focus on processing tasks
void* workerWrapper(void* arg) {
    Worker* worker = static_cast<Worker*>(arg); //passes through a pointer to the worker instnace

    while (!worker->stopRequested) { //manual stopping method
        Task task = gettask(worker->inputQueue);

        if (task.isEOS) {//if this is the terminating task
            break;
        }

        if(task.isValid) {
            Result result;
            void* output = worker->workerFunction(task.data);
            result.data = output;
            puttask(worker->outputQueue, result);
            task.isValid = false; //now that task has been completed, we invalidate it,
        } else {
            sched_yield(); //yield if no task was fetched, allows to reduce busy waiting
        }

    }
    return nullptr;
}


ThreadSafeQueue<Result> Farm::process(ThreadSafeQueue<Task>& tasks) {
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

    ThreadSafeQueue<Result> results;

    //collection of results from each workers queue
    for (int i = 0; i < numOfWorkers; ++i) {
        Result result;
        while(dequeueResult(i, result)) {
            results.enqueue(result);
        }
    }
    return results;
}


//uses round-robin
void Farm::distributeTasks(ThreadSafeQueue<Task>& queue) {

    while (!queue.empty()) {
        Task task;
        queue.dequeue(task);
        workers[currentWorker].inputQueue.enqueue(task);
        //move to next worker
        currentWorker = (currentWorker + 1) % numOfWorkers;
    }
}

// Add a task to the output queue for a specific worker
void Farm::enqueueResult(int workerId, const Result& result) {
    std::cout << "result has been pushed" << std::endl;
    workers[workerId].outputQueue.enqueue(result);
}

// Remove and retrieve a task from the output queue for a specific worker
bool Farm::dequeueResult(int workerId, Result& result) {
    return workers[workerId].outputQueue.dequeue(result);

}

//Instead of making an EOS flag directly to the worker, creating an EOS task instead allows tasks to finish
//without abruptly shutting down and extra synchronisation
void Farm::signalEOS() {
    ThreadSafeQueue<Task> eosTasks;
    //EOS task
    Task eosTask(nullptr, true);
    for(int i = 0; i < numOfWorkers; ++i) {
        eosTasks.enqueue(eosTask);
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





