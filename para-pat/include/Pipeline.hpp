#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "Stage.hpp"
#include <vector>
#include <queue>

typedef void* (*WorkerFunction)(void*);

class Pipeline {
public:
    std::vector<Stage<Task>*> stages; //vector of pointers to the stages

    void addStage(Stage<Task>* stage);

    ThreadSafeQueue<Result> execute(ThreadSafeQueue<Task>& inputQueue) ;

    //destructor (for clean up)
    ~Pipeline();

    void terminate();

    std::vector<Stage<Task>*> getStages();

};

#endif