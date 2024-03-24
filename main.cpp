#include "include/Farm.hpp"


Task gettask(std::queue<Task>& inputQueue, pthread_mutex_t& inputMutex) {
    //attempt to get a task from the input queue
    pthread_mutex_lock(&inputMutex);
    Task task;
    if (!inputQueue.empty()) {
        task = inputQueue.front();
        inputQueue.pop();
        task.isValid = true; //mark as valid task
    }
    pthread_mutex_unlock(&inputMutex);
    return task; //return task that is not valid
}

// Result doWork(const Task& task) {
//     Result result;
//     result.result = task.function(task.arg);
//     return result;
// }

void puttask(std::queue<Result>& outputQueue, pthread_mutex_t& outputMutex, const Result& result) {
    pthread_mutex_lock(&outputMutex);
    outputQueue.push(result);
    pthread_mutex_unlock(&outputMutex);
}


//The worker wrapper abstracts the queuing logic, ensuring thread-safety and allowing us to focus on processing tasks
void* workerWrapper(void* arg) {
    Worker* worker = static_cast<Worker*>(arg); //passes through a pointer to the worker instnace


    while (!worker->stopRequested) { //manual stopping method
        Task task = gettask(worker->inputQueue, worker->inputMutex);

        if (task.isEOS) {//if this is the terminating task
            break;
        }

        if(task.isValid) {
            Result result = task.function(task.arg);
            puttask(worker->outputQueue, worker->outputMutex, result);
            task.isValid = false; //now that task has been completed, we invalidate it,
        } else {
            sched_yield(); //yield if no task was fetched, allows to reduce busy waiting
        }

    }

    return nullptr;
}


Result increment(void* arg) {
    int* pValue = static_cast<int*>(arg);

    Result res;
    if (pValue) {
        ++(*pValue);
    }
    
    res.data = pValue;
    return res;
}



int main() {
    int numOfWorkers = 4;
    Farm farm(numOfWorkers, workerWrapper);
    
    int lol = 0;
    //submit tasks to specific workers
    for(int i = 0; i < numOfWorkers; ++i) {
        Task task(increment, &lol);
        farm.submitTask(i, task);
    }

    // sleep(2);

    farm.signalEOS(); //sends EOS task to all workers to halt processing 
    farm.stopWorkers();

    //collect the results from each worker's output queue
    std::vector<Result> allResults;
    for (int i = 0; i < numOfWorkers; ++i) {
        Result result;
        while (farm.dequeueResult(i, result)) {
            allResults.push_back(result);
        }
    }

    for (const auto& res : allResults) {
        int* result = static_cast<int*>(res.data);
        std::cout << "Result: " << *result << std::endl;
    }

    std::cout << "hello" << std::endl;



    return 0;
}