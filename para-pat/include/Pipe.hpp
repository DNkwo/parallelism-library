#ifndef PIPE_HPP
#define PIPE_HPP

#include "Stage.hpp"


class Pipe : public Stage<Task> {

public:
    Pipe(WorkerFunction workerFunction);

    ~Pipe();
    
};

#endif