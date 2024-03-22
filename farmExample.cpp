#include <iostream>
#include <pthread.h>
#include <unistd.h>

//struct to pass arguments to thread
struct ThreadData {
    int threadId;
};

//worker function, executed by each thread
void* workerFunction(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    std::cout << "Worker " << data->threadId << " is executing." << std::endl;
    //stimulate some work
    sleep(1);
    return nullptr;
}

void createFarm(int numOfWorkers, void* (*workerFunction)(void*)) {
    pthread_t* threads = new pthread_t[numOfWorkers];
    ThreadData* threadData = new ThreadData[numOfWorkers];

    //create worker threads
    for (int i = 0; i < numOfWorkers; ++i) {
        threadData[i].threadId = i + 1;
        pthread_create(&threads[i], nullptr, workerFunction, &threadData[i]);
    }

    //wait for all worker threads to finish
    for (int i = 0; i < numOfWorkers; ++i) {
        pthread_join(threads[i], nullptr);
    }

    delete[] threads;
    delete[] threadData;
}

int main() {
    int numOfWorkers = 4;
    createFarm(numOfWorkers, workerFunction);

    return 0;
}