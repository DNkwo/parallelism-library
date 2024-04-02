#ifndef STAGE_HPP
#define STAGE_HPP

#include "ThreadSafeQueue.hpp"
#include <queue>


//define worker function type
typedef void* (*WorkerFunction)(void*);


struct Result {
    void* data;

    // Constructor
    Result() : data(nullptr) {}
};

struct Task {
    void* data = nullptr; //stores the data of the task
    bool isValid = true; //by default
    bool isEOS = false;  //End of Stream task
    bool isShutdown = false; //Shutdown task

    //default construct
    Task() : data(nullptr) {}

    //constructors
    Task(void* data) 
    :  data(data) {}

    Task(void* data, bool isEOS) 
    :  data(data), isEOS(isEOS) {}

};

class Worker {
public:
    size_t id = 0;
    //thread-safety implementation of queues (using pthread mutex)
    ThreadSafeQueue<Task>* inputQueue; 
    std::vector<ThreadSafeQueue<Task>*> outputQueues; //workers can potentially hold multiple output queues for round-robin

    pthread_t thread;

    WorkerFunction workerFunction = nullptr;
    // we use this flag to singal worker to stop (the thread, regardless of state of task queue)
    volatile bool stopRequested = false; // (volatile ensures threads do not miss updates from main thread)

    Worker() : stopRequested(false) {
        inputQueue = new ThreadSafeQueue<Task>;
    }

    ~Worker() {
        //de allocate the input and output queues
        delete inputQueue;
        for (ThreadSafeQueue<Task>* outputQueue : outputQueues) {
            if (outputQueue != nullptr) {
                delete outputQueue;
            }
        }
    }

    void run() { //this is the worker's main function
        size_t nextWorkerIndex = 0;
        while (!stopRequested) {
            Task task;
            if (inputQueue->dequeue(task)) {
                if (task.isEOS) {
                    //enqueue the EOS task to all workers in the next stage
                    for (ThreadSafeQueue<Task>* outputQueue : outputQueues) {
                        outputQueue->enqueue(task);
                    }
                    continue;
                }

                if (task.isShutdown) {
                    stopRequested = true;
                    // enqueue the shutdown task to all workers in the next stage
                    for (ThreadSafeQueue<Task>* outputQueue : outputQueues) {
                        outputQueue->enqueue(task);
                    }
                    break;
                }

                if (task.isValid) {
                    Task result;
                    void* output = workerFunction(task.data);
                    result.data = output;
                    
                    //enqueue the result task to the next worker using round-robin
                    outputQueues[nextWorkerIndex]->enqueue(result);
                    nextWorkerIndex = (nextWorkerIndex + 1) % outputQueues.size();
                }
            } else {
                sched_yield(); //yield if no task was fetched to reduce busy waiting
            }
        }

    // Propagate the shutdown task to the next stage
        while (!inputQueue->empty()) {
            Task task;
            if (inputQueue->dequeue(task)) {
                if (task.isShutdown) {
                    for (ThreadSafeQueue<Task>* outputQueue : outputQueues) {
                        outputQueue->enqueue(task);
                    }
                    break;
                }
            }
        }
    }

};

template<typename Task>
class Stage {
protected:
    WorkerFunction workerFunction;
    //stage status flags
    bool eosReceived = false;
    bool isShutdown = false;
    std::vector<Worker*> workers;
public:

    Stage(WorkerFunction workerFunction = nullptr) : workerFunction(workerFunction) {}
    virtual ~Stage() = default; // virtual destuctor for controlled clean up

    //this is the static function 'workerWrapper' uses for the worker threads
    static void* workerWrapper(void* arg) {
        Worker* worker = static_cast<Worker*>(arg);
        worker->run(); 
        return worker;
    }

    //connects the workers to the next stage
    void connectWorkers(const std::vector<Worker*>& nextStageWorkers) {
        for (Worker* worker : workers) {
            worker->outputQueues.clear();
            for (Worker* nextWorker : nextStageWorkers) {
                worker->outputQueues.push_back(nextWorker->inputQueue);
            }
        }
    }

    // get the workers of the stage
    std::vector<Worker*> getWorkers() {
        return workers;
    }
 
};

#endif
