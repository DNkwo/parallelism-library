#ifndef STAGE_HPP
#define STAGE_HPP

#include "ThreadSafeQueue.hpp"

#include <queue>

//define worker function type
typedef void* (*WorkerFunction)(void*);

struct Result {
    void* data;
};

struct Task {
    void* data = nullptr;
    bool isValid = true; //by default
    bool isEOS = false;


    //default construct
    Task() : data(nullptr), isValid(true), isEOS(false) {}

    //constructors
    Task(void* data) 
    :  data(data), isValid(true), isEOS(false) {}


    Task(void* data, bool isEOS) 
    :  data(data), isValid(true), isEOS(isEOS) {}
};

template<typename Task, typename Result>
class Stage {
protected:
    WorkerFunction workerFunction;
public:

    Stage(WorkerFunction workerFunction = nullptr) : workerFunction(workerFunction) {}

    virtual ThreadSafeQueue<Result> process(ThreadSafeQueue<Task>& input) = 0; // pure virtual function to process the pattern
    virtual ~Stage() = default; // virtual destuctor for controlled clean up
};

#endif
