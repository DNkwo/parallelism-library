#ifndef PIPE_HPP
#define PIPE_HPP

#include "Stage.hpp"

class Pipe : public Stage<Task, Result> {

private:

public:
    ThreadSafeQueue<Result> process(ThreadSafeQueue<Task> &input) override {

    }
};

#endif