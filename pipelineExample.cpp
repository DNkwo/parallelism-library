#include <iostream>
#include <pthread.h>
#include <queue>
#include <vector>

struct PipelineStage {
    std::queue<int> queue;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    bool finished = false; //indicates if stage is finished processing
};

void* stageFunction(void* arg) {
    auto* stage = static_cast<PipelineStage*>(arg);

    //processing loop
    while (true) {
        pthread_mutex_lock(&stage->mtx);

        //check if stage is finished and queue is empty
        if (stage->finished && stage->queue.empty()) {
            pthread_mutex_unlock(&stage->mtx);
            break;
        }

        if (!stage->queue.empty()) {
            //processing data
            int data = stage->queue.front();
            stage->queue.pop();
            pthread_mutex_unlock(&stage->mtx);

            data++;

            std::cout << "Processed data: " << data << std::endl;
        } else {
            pthread_mutex_unlock(&stage->mtx);
            // if no data, we use sleep to prevent busy-waiting
            timespec ts = {0, 1000000};
            nanosleep(&ts, nullptr);
        }
    }

    return nullptr;
}

void initPipeline(std::vector<PipelineStage>& stages, std::vector<pthread_t>& threads) {
    for (size_t i = 0; i < threads.size(); ++i) {
        pthread_create(&threads[i], nullptr, stageFunction, &stages[i]);
    }
}


int main() {
    const size_t numStages = 3;
    std::vector<PipelineStage> stages(numStages);
    std::vector<pthread_t> threads(numStages);

    initPipeline(stages, threads);

    //simulate feeding data into the first stage
    pthread_mutex_lock(&stages[0].mtx);
    for (int i = 0; i < 10; ++i) {
        stages[0].queue.push(i);
    }
    stages[0].finished = true;
    pthread_mutex_unlock(&stages[0].mtx);

    //wait for all threads to complete
    for (auto& t : threads) {
        pthread_join(t, nullptr);
    }

    return 0;
}