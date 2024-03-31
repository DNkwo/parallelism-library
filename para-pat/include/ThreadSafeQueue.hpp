#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#include <pthread.h>
#include <queue>
#include <iostream>

//thread-safe queue (pthread_mutex wrapper around std::queue)
template<typename T>
class ThreadSafeQueue {
private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::queue<T> queue;

public:
    //constructor
    ThreadSafeQueue() {
        int result = pthread_mutex_init(&mutex, nullptr);
        if (result != 0) {
            std::cerr << "Failed to initialize mutex: " << result << std::endl;
        }
        result = pthread_cond_init(&cond, nullptr);
        if (result != 0) {
            std::cerr << "Failed to initialize condition variable: " << result << std::endl;
        }
    }

    ~ThreadSafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    //add item to queue
    void enqueue(const T& item) {
        pthread_mutex_lock(&mutex);
        queue.push(item);
        pthread_cond_signal(&cond); // Signal the condition variable
        pthread_mutex_unlock(&mutex);
    }

    bool dequeue(T& item) {
        pthread_mutex_lock(&mutex);
        if (queue.empty()) {
            pthread_mutex_unlock(&mutex);
            return false; // queue empty so no item retireved
        }
        item = queue.front();
        queue.pop();
        pthread_mutex_unlock(&mutex);
        return true; // item succesfully retrieved
    }

    //check if the queue is empty (additional thread safety protection)
    bool empty() {
        pthread_mutex_lock(&mutex);
        bool isEmpty = queue.empty();
        pthread_mutex_unlock(&mutex);
        return isEmpty;
    }

    size_t size() {
        pthread_mutex_lock(&mutex);
        size_t queueSize = queue.size();
        pthread_mutex_unlock(&mutex);
        return queueSize;
    }


};

#endif