#include "include/Farm.hpp"

//worker function, executed by each thread
// void* workerFunction(void* arg) {
//     Worker* worker = static_cast<Worker*>(arg);

//     while (!worker->stopRequested) {
//         Task task;
//         bool hasTask = false;

//         //attempt to get a task from the input queue
//         pthread_mutex_lock(&worker->inputMutex);
//         if (!worker->inputQueue.empty()) {
//             task = worker->inputQueue.front();
//             worker->inputQueue.pop();
//             hasTask = true;
//         }
//         pthread_mutex_unlock(&worker->inputMutex);

//         if (hasTask) {
//             // Process the task
//             Result result;
//             result.value = task.value + 1;
//             pthread_mutex_lock(&worker->outputMutex);
//             worker->outputQueue.push(result);
//             pthread_mutex_unlock(&worker->outputMutex);
//         } else if (worker->stopRequested) {
//             //exit loop if no task found and stopping is requested
//             break;
//         } else {
//             sched_yield(); //yield the processor, to reduce busy-waiting overhead
//         }
//     }

//     return nullptr;
// }


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

Result doWork(const Task& task) {
    Result result;
    result.value = task.value + 1;
    return result;
}

void puttask(std::queue<Result>& outputQueue, pthread_mutex_t& outputMutex, const Result& result) {
    pthread_mutex_lock(&outputMutex);
    outputQueue.push(result);
    pthread_mutex_unlock(&outputMutex);
}

void* workerWrapper(void* arg) {
    Worker* worker = static_cast<Worker*>(arg);

    while (!worker->stopRequested) { //manual stopping method
        Task task = gettask(worker->inputQueue, worker->inputMutex);

        if (task.isEOS) {//if this is the terminating task
            break;
        }

        if(task.isValid) {
            Result result = doWork(task);
            puttask(worker->outputQueue, worker->outputMutex, result);
            task.isValid = false; //now that task has been completed, we invalidate it,
        } else {
            sched_yield(); //yield if no task was fetched, allows to reduce busy waiting
        }

    }

    return nullptr;
}



int main() {
    int numOfWorkers = 2;
    Farm farm(numOfWorkers, workerWrapper);
    
    //submit tasks to specific workers
    for(int i = 0; i < numOfWorkers; ++i) {
        Task task;
        farm.submitTask(i, task);
    }

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
        std::cout << "Result: " << res.value << std::endl;
    }



    return 0;
}