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

    Farm(int numOfWorkers, WorkerFunction workerFunction) : Stage<Task>(workerFunction), numOfWorkers(numOfWorkers) {    

        workers.resize(numOfWorkers);

        //initialsie workers, assign id and the worker function to each worker
        for (int i = 0; i < numOfWorkers; ++i) {
            workers[i] = new Worker();
            workers[i]->id = i + 1;
            workers[i]->workerFunction = workerFunction;
        }

        //create worker threads
        for (int i = 0; i < numOfWorkers; ++i) {
            pthread_t thread;
            pthread_create(&workers[i]->thread, nullptr, workerWrapper, workers[i]); //passes the worker[i] as an argument to the workerFunction
        }

    }

    ~Farm(){
        for (int i = 0; i < numOfWorkers; ++i) {
            pthread_join(workers[i]->thread, nullptr);
        }
    }

};

#endif