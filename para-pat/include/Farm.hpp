#ifndef FARM_HPP
#define FARM_HPP

#include "Stage.hpp"
#include "ThreadSafeQueue.hpp"

#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>


class Farm : public Stage<Task> {
private:
    size_t numOfWorkers;
public:
    Farm(size_t numOfWorkers, WorkerFunction workerFunction);

    ~Farm();

};

#endif