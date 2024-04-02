#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "Stage.hpp"
#include <vector>
#include <queue>

typedef void* (*WorkerFunction)(void*); //we use a type defintion for worker function pointer

class Pipeline {
public:
    std::vector<Stage<Task>*> stages; //vector of pointers to the stages in the pipeline

    void addStage(Stage<Task>* stage); //adds a stage to the pipeline

    ThreadSafeQueue<Result> execute(ThreadSafeQueue<Task>& inputQueue) ; //excutes the pipeline with in an input queue

    void connectStages(); //connects the stages in the pipeline

    void terminate(); //terminates the pipeline

    std::vector<Stage<Task>*> getStages(); //function to ge the stages in the pipeline

};

#endif