#include <stdio.h>
#include "para-pat/include/Farm.hpp"
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
   
    // Create a pipeline with two stages
    Pipe stage1(payload1);
    Pipe stage2(payload2);

    // Create a pipeline manager and add the stages
    StageManager pipelineManager;
    pipelineManager.addStage(&stage1);
    pipelineManager.addStage(&stage2);


    //add the tasks
    ThreadSafeQueue<Task> inputQueue;
    // for (int i = 0; i < 3; i++) {
    //     int* taskData = new int(i);
    //     Task task = new Task(taskData);
    //     pipelineManager.addTask(task);
    // }

    // Execute the pipeline
    ThreadSafeQueue<Result> outputQueue = pipelineManager.execute(inputQueue);
    pipelineManager.terminate();

    // Process the results
    while (!outputQueue.empty()) {
      std::cout << "Result: " << std::endl;
      Result result;
      if (outputQueue.dequeue(result)) {
          int* rs = static_cast<int*>(result.data);
          std::cout << "Result: " << *rs << std::endl;
          delete rs; //prevent memory leaks
      }
    }

  

    return 0;
}