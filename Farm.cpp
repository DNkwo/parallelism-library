#include "include/Farm.hpp"
#include <iostream>
#include <unistd.h>
#include <queue>
#include <vector>

Farm::Farm(int numOfWorkers, WorkerFunction workerFunction)
    : Stage<Task>(workerFunction), numOfWorkers(numOfWorkers) {
    workers.resize(numOfWorkers);

    //assign id and the worker function to each worker
    for (int i = 0; i < numOfWorkers; ++i) {
        workers[i].id = i + 1;
        workers[i].workerFunction = workerFunction;
    }

     //create worker threads
    for (int i = 0; i < numOfWorkers; ++i) {
        pthread_t thread;
        pthread_create(&thread, nullptr, workerWrapper, &workers[i]); //passes the worker[i] as an argument to the workerFunction
        threads.push_back(thread);
    }

}

Farm::~Farm() {
    for (auto& t : threads) {
        if (t) {
            pthread_join(t, nullptr); //ensure all threads are joined
        }
    }
}

ThreadSafeQueue<Task> Farm::process(ThreadSafeQueue<Task>& input) {
    //preparing input
    distributeTasks(input);

    joinThreads();

    ThreadSafeQueue<Task> results;

    // //collection of results from each workers queue
    for (int i = 0; i < numOfWorkers; ++i) {
        Task result;
        while(dequeueResult(i, result)) {
            results.enqueue(result);
        }
    }

    return results;
}


//uses round-robin
void Farm::distributeTasks(ThreadSafeQueue<Task>& tasks) {

    while (!tasks.empty()) {
        Task task;
        tasks.dequeue(task);

        if(task.isEOS && !eosReceived) {
            //first EOS task has been received, so we distribute EOS to all workers
            eosReceived = true;
            signalEOS();
        }
        else if (!eosReceived) { //normal task distribution (round-robin)
            workers[currentWorker].inputQueue.enqueue(task);
            currentWorker = (currentWorker + 1) % numOfWorkers;
        }

        //if eosReceived true and task not EOS, we ignore this method as workers are shutting down
    }
}

// Add a task to the output queue for a specific worker
void Farm::enqueueResult(int workerId, const Task& result) {
    std::cout << "result has been pushed" << std::endl;
    workers[workerId].outputQueue.enqueue(result);
}

// Remove and retrieve a task from the output queue for a specific worker
bool Farm::dequeueResult(int workerId, Task& result) {
    return workers[workerId].outputQueue.dequeue(result);

}

//Instead of making an EOS flag directly to the worker, creating an EOS task instead allows tasks to finish
//without abruptly shutting down and extra synchronisation
void Farm::signalEOS() {
    for(Worker& worker : workers) {
        Task eosTask(nullptr, true); //create EOS task
        worker.inputQueue.enqueue(eosTask);
    }
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





