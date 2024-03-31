#include <stdio.h>
#include "para-pat/include/Farm.hpp"
#include "para-pat/include/Pipeline.hpp"
#include "para-pat/include/Pipe.hpp"
#include "para-pat/include/StageManager.hpp"

int fib(int n) {
    int i, Fnew, Fold, temp, ans;

    Fnew = 1;
    Fold = 0;
    for (i = 2; i <= n; i++) {
        temp = Fnew;
        Fnew = Fnew + Fold;
        Fold = temp;
    }
    ans = Fnew;
    return ans;
}

void* payload1(void* arg) {
    int i = *static_cast<int*>(arg);
    int* result = new int(fib(5));
    return result;
}

void* payload2(void* arg) {
    int i = *static_cast<int*>(arg);
    int* result = new int(fib(5));
    return result;
}

int main() {
    StageManager manager;

    Pipe pipe1(payload1);
    Pipe pipe2(payload2);

    manager.addStage(&pipe1);
    manager.addStage(&pipe2);

    ThreadSafeQueue<Task> inputQueue;
    for (int i = 0; i < 2; i++) {
        int* taskData = new int(i);
        inputQueue.enqueue(Task(taskData));
    }

    ThreadSafeQueue<Result> output = manager.execute(inputQueue);
    manager.terminate();

    while (!output.empty()) {
        Result result;
        if (output.dequeue(result)) {
            int* res2 = static_cast<int*>(result.data);
            printf("%d\n", *res2);
            delete res2;
        }
    }

    return 0;
}