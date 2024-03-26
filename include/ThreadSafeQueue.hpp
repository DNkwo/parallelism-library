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
    std::queue<T> queue;
public:
    //constructor
    ThreadSafeQueue() {
        pthread_mutex_init(&mutex, nullptr);
    }

    ~ThreadSafeQueue() {
        pthread_mutex_destroy(&mutex);
    }

    //add item to queue
    void enqueue(const T& item) {
        pthread_mutex_lock(&mutex);
        queue.push(item);
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


};

#endif