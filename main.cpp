#include "include/Farm.hpp"


void* increment(void* arg) {
    int* pValue = static_cast<int*>(arg);

    Result res;
    if (pValue) {
        ++(*pValue);
    }
    
    res.data = pValue;

    return nullptr;
}



int main() {

    int lol = 0;
    //Preperation of tasks
    std::vector<Task> tasks;
    for(int i = 0; i < 10; ++i) {
        Task task(&lol);
        tasks.push_back(task);
    }

    Farm farm(4, increment);
    farm.addTasksAndProcess(tasks);
    
    // //submit tasks to specific workers
    // for(int i = 0; i < numOfWorkers; ++i) {
    //     Task task(increment, &lol);
    //     farm.submitTask(i, task);
    // }


    // farm.signalEOS(); //sends EOS task to all workers to halt processing 

    //collect the results from each worker's output queue
    // std::vector<Result> allResults;
    // for (int i = 0; i < numOfWorkers; ++i) {
    //     Result result;
    //     while (farm.dequeueResult(i, result)) {
    //         allResults.push_back(result);
    //     }
    // }

    // for (const auto& res : allResults) {
    //     int* result = static_cast<int*>(res.data);
    //     std::cout << "Result: " << *result << std::endl;
    // }

    std::cout << "hello" << std::endl;



    return 0;
}