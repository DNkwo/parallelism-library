#ifndef PIPE_HPP
#define PIPE_HPP

#include "Stage.hpp"


class Pipe : public Stage<Task> {

private:
    Worker worker;
    pthread_t thread;
public:

    Pipe(WorkerFunction workerFunction) : Stage<Task>(workerFunction){
        //initialise worker
        worker.workerFunction = workerFunction;

        //start thread
        pthread_create(&this->thread, nullptr, workerWrapper, &worker);
    }

    ~Pipe(){
        pthread_join(this->thread, nullptr);
    } //destructor

    ThreadSafeQueue<Task> process(ThreadSafeQueue<Task> &input) override {

        while (!input.empty()) {
            Task task;
            input.dequeue(task);

            if(task.isShutdown) {
                isShutdown = true;
                worker.inputQueue.enqueue(task);
            }
            else if(task.isEOS && !eosReceived) {
                //first EOS task has been received, so we distribute EOS to all workers
                eosReceived = true;
                signalEOS();
            }
            else if (!eosReceived) { //normal task distribution 
                worker.inputQueue.enqueue(task);
            }
        //if eosReceived true and task not EOS, we ignore this method as worker is shutting
        }

        waitForTaskCompletion();

        ThreadSafeQueue<Task> results;
        Task result;

        while(worker.outputQueue.dequeue(result)) {
            results.enqueue(result);
        } 

        return results;
    }

    void waitForTaskCompletion() {
        while((worker.isProcessing || worker.taskCounter != 0 || !worker.eosReceived) && !isShutdown){
            //probably put some sleep function here
        }
    }

    void signalEOS() override {
        Task eosTask(nullptr, true);
        worker.inputQueue.enqueue(eosTask);
    }

    void signalShutdown() override {
        Task signalShutdown(nullptr);
        signalShutdown.isShutdown = true;
    }

    
};

#endif