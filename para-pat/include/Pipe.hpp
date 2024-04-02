#ifndef PIPE_HPP
#define PIPE_HPP

#include "Stage.hpp"


//child of Stage 
class Pipe : public Stage<Task> {

public:
    Pipe(WorkerFunction workerFunction); //takes a worker function as parameter

    ~Pipe();
    
};

#endif