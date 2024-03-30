#include "include/Pipe.hpp"

Pipe::Pipe(WorkerFunction workerFunction) : Stage<Task>(workerFunction){
    workers.push_back(new Worker());
    //initialize worker
    workers[0]->workerFunction = workerFunction;

    //start thread
    pthread_create(&workers[0]->thread, nullptr, workerWrapper, workers[0]);
}

Pipe::~Pipe(){
    // pthread_join(workers[0]->thread, nullptr);
}