#ifndef PIPE_HPP
#define PIPE_HPP

#include "Stage.hpp"


class Pipe : public Stage<Task, Result> {

private:
    Worker worker;
    pthread_t thread;
public:

    Pipe(WorkerFunction workerFunction) : Stage<Task, Result>(workerFunction){
        //initialise worker
        worker.workerFunction = workerFunction;

        //start thread
        pthread_create(&this->thread, nullptr, workerWrapper, &worker);
    }

    ~Pipe(){
        pthread_join(this->thread, nullptr);
    } //destructor

    ThreadSafeQueue<Result> process(ThreadSafeQueue<Task> &input) override {

        while (!input.empty()) {
            Task task;
            input.dequeue(task);

            if(task.isEOS && !eosReceived) {
                //first EOS task has been received, so we distribute EOS to all workers
                eosReceived = true;
                signalEOS();
            }
            else if (!eosReceived) { //normal task distribution (round-robin)
                worker.inputQueue.enqueue(task);
            }
        //if eosReceived true and task not EOS, we ignore this method as worker is shutting
        }

        while(!checkWorkerCompletion()) {
            // std::cout << worker.isProcessing << std::endl;
            //probably put some sleep function here
        }

        ThreadSafeQueue<Result> results;
        Result result;

        while(worker.outputQueue.dequeue(result)) {
            results.enqueue(result);
        } 

        return results;
    }

    bool checkWorkerCompletion() {
        return !worker.isProcessing && worker.taskCounter == 0 && worker.eosReceived;
    }

    void signalEOS() override {
        Task eosTask(nullptr, true);
        worker.inputQueue.enqueue(eosTask);
    }

    
};

#endif