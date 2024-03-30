#include "include/Farm.hpp"

Farm::Farm(size_t numOfWorkers, WorkerFunction workerFunction) : Stage<Task>(workerFunction), numOfWorkers(numOfWorkers) {    
    workers.resize(numOfWorkers);

    //initialize workers, assign id and the worker function to each worker
    for (size_t i = 0; i < numOfWorkers; ++i) {
        workers[i] = new Worker();
        workers[i]->id = i + 1;
        workers[i]->workerFunction = workerFunction;
    }

    //create worker threads
    for (size_t i = 0; i < numOfWorkers; ++i) {
        pthread_create(&workers[i]->thread, nullptr, workerWrapper, workers[i]); //passes the worker[i] as an argument to the workerFunction
    }
}

Farm::~Farm(){
    // for (size_t i = 0; i < numOfWorkers; ++i) {
    //     pthread_join(workers[i]->thread, nullptr);
    // }
}