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
    void* data = nullptr;
    bool isValid = true; //by default
    bool isEOS = false;
    bool isShutdown = false;

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
    ThreadSafeQueue<Task>* outputQueue;

    WorkerFunction workerFunction = nullptr;

    // we use this flag to singal worker to stop (the thread, regardless of state of task queue)
    volatile bool stopRequested = false; // (volatile ensures threads do not miss updates from main thread)


    Worker() : stopRequested(false) {

        //initiate outputQueue
        outputQueue = new ThreadSafeQueue<Task>;
    }

    ~Worker() {}

    void run() {
        while (!stopRequested) {
            Task task;
            if (inputQueue->dequeue(task)) { //maybe implementing blocking until task is available?
                if (task.isEOS) { // Check for End-Of-Stream task (end of batch)
                    outputQueue->enqueue(task); //enqueues the EOS task for the next stage
                    continue; //continue to process next batch
                }

                if (task.isShutdown) {
                    stopRequested = true; //stops thread loop
                    outputQueue->enqueue(task); //enqueues shutdown task for next stages
                    continue;
                }

                if(task.isValid) {
                    std::cout << this->id << std::endl;
                    Task result;
                    void* output = workerFunction(task.data);
                    result.data = output;
                    outputQueue->enqueue(result);
                }
            } else {
                sched_yield(); // Yield if no task was fetched, to reduce busy waiting
            }

        }
    }

};

template<typename Task>
class Stage {
protected:
    WorkerFunction workerFunction;
    bool eosReceived = false;
    bool isShutdown = false;
    std::vector<Worker*> workers;
    ThreadSafeQueue<Task>* inputQueue;
public:

    Stage(WorkerFunction workerFunction = nullptr) : workerFunction(workerFunction) {}

    virtual ThreadSafeQueue<Task> process(ThreadSafeQueue<Task>& input) = 0; // pure virtual function to process the pattern
    virtual void signalEOS() = 0; //signalling EOS tasks
    // virtual void signalShutdown() = 0; //signalling shutdown tasks
    virtual ~Stage() = default; // virtual destuctor for controlled clean up

    static void* workerWrapper(void* arg) {
        Worker* worker = static_cast<Worker*>(arg);
        worker->run(); 
        return nullptr;
    }

    ThreadSafeQueue<Task>* getInputQueue() {
        return this->inputQueue;
    }

    std::vector<Worker*> getWorkers() {
        return workers;
    }
 
};

#endif
