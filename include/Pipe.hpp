#ifndef PIPE_HPP
#define PIPE_HPP

#include "Stage.hpp"


class Pipe : public Stage<Task> {

public:

    Pipe(WorkerFunction workerFunction) : Stage<Task>(workerFunction){
        // this->inputQueue = new ThreadSafeQueue<Task>;
        
        workers.push_back(new Worker());
        //initialise worker
        workers[0]->workerFunction = workerFunction;

        //start thread
        pthread_create(&workers[0]->thread, nullptr, workerWrapper, workers[0]);
    }

    ~Pipe(){
        pthread_join(workers[0]->thread, nullptr);
    } //destructor
    
};

#endif