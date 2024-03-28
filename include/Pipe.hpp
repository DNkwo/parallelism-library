#ifndef PIPE_HPP
#define PIPE_HPP

#include "Stage.hpp"


class Pipe : public Stage<Task> {

private:
    pthread_t thread;
public:

    Pipe(WorkerFunction workerFunction) : Stage<Task>(workerFunction){
        this->inputQueue = new ThreadSafeQueue<Task>;
        
        
        workers.push_back(new Worker());
        //initialise worker
        workers[0]->workerFunction = workerFunction;

        // //assigns the worker's input queue to be the same as stage input queue
        workers[0]->inputQueue = this->inputQueue;

        //start thread
        pthread_create(&this->thread, nullptr, workerWrapper, workers[0]);
    }

    ~Pipe(){
        pthread_join(this->thread, nullptr);
    } //destructor

    ThreadSafeQueue<Task> process(ThreadSafeQueue<Task> &input) override {

        // while (!input.empty()) {
        //     Task task;
        //     input.dequeue(task);

        //     if(task.isShutdown) {
        //         isShutdown = true;
        //         workers[0].inputQueue->enqueue(task);
        //     }
        //     else if(task.isEOS && !eosReceived) {
        //         //first EOS task has been received, so we distribute EOS to all workers
        //         eosReceived = true;
        //         signalEOS();
        //     }
        //     else if (!eosReceived) { //normal task distribution 
        //         workers[0].inputQueue->enqueue(task);
        //     }
        // //if eosReceived true and task not EOS, we ignore this method as worker is shutting
        // }

        // waitForTaskCompletion();

        ThreadSafeQueue<Task> results;
        // Task result;

        // while(workers[0].outputQueue.dequeue(result)) {
        //     results.enqueue(result);
        // } 

        return results;
    }

    void signalEOS() override {
        Task eosTask(nullptr, true);
        workers[0]->inputQueue->enqueue(eosTask);
    }

    // void signalShutdown() override {
    //     Task signalShutdown(nullptr);
    //     signalShutdown.isShutdown = true;
    // }



    
};

#endif