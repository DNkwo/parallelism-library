#ifndef STAGE_HPP
#define STAGE_HPP

#include "ThreadSafeQueue.hpp"
#include <queue>


//define worker function type
typedef void* (*WorkerFunction)(void*);


// struct Result {
//     void* data;

//     // Constructor
//     Result() : data(nullptr) {}
// };

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

class Worker {
public:
    size_t id = 0;
    //thread-safety implementation of queues (using pthread mutex)
    ThreadSafeQueue<Task> inputQueue; 
    ThreadSafeQueue<Task> outputQueue;

    WorkerFunction workerFunction = nullptr;

    // we use this flag to singal worker to stop (the thread, regardless of state of task queue)
    volatile bool stopRequested = false; // (volatile ensures threads do not miss updates from main thread)


    //for synchronsiation
    bool isProcessing = false;
    int taskCounter = 0;
    bool eosReceived = false;

    Worker() : stopRequested(false) {}

    ~Worker() {}

    void run() {
        while (!stopRequested) {
            Task task;
            if (inputQueue.dequeue(task)) { //maybe implementing blocking until task is available?
                if (task.isEOS) { // Check for End-Of-Stream task (end of batch)
                    eosReceived = true;
                    continue; //continue to process next batch
                }

                if(task.isValid) {
                    isProcessing = true;
                    taskCounter++;
                    Result result;
                    void* output = workerFunction(task.data);
                    result.data = output;
                    outputQueue.enqueue(result);
                    std::cout << inputQueue.size() << "," << outputQueue.size() << std::endl;
                    taskCounter--;
                    isProcessing = false;
                }
            } else {
                sched_yield(); // Yield if no task was fetched, to reduce busy waiting
            }

            //after an EOS, we reset and prepare for next batch of tasks
            if(eosReceived && inputQueue.empty() && taskCounter == 0) {
                eosReceived = false;
            }
        }
    }

};

template<typename Task, typename Result>
class Stage {
protected:
    WorkerFunction workerFunction;
    bool eosReceived = false;
public:

    Stage(WorkerFunction workerFunction = nullptr) : workerFunction(workerFunction) {}

    virtual ThreadSafeQueue<Result> process(ThreadSafeQueue<Task>& input) = 0; // pure virtual function to process the pattern
    virtual void signalEOS() = 0; //signalling EOS tasks
    virtual ~Stage() = default; // virtual destuctor for controlled clean up


    static void* workerWrapper(void* arg) {
        Worker* worker = static_cast<Worker*>(arg);
        worker->run(); 
        return nullptr;
    }
    
 
};

#endif
