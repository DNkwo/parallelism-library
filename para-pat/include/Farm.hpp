#ifndef FARM_HPP
#define FARM_HPP

#include "Stage.hpp"
#include "ThreadSafeQueue.hpp"

#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>


//child of stage
class Farm : public Stage<Task> {
private:
    size_t numOfWorkers; //num of workers threads in the farm
public:
//takes number of workers and a worker function as parameters
    Farm(size_t numOfWorkers, WorkerFunction workerFunction);

    ~Farm(); //destructor

};

#endif