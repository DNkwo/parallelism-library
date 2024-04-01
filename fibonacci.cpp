#include <stdio.h>
#include "para-pat/include/Farm.hpp"
#include "para-pat/include/Pipeline.hpp"
#include "para-pat/include/Pipe.hpp"
#include <sys/time.h>

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
    int num = *static_cast<int*>(arg);
    int* result = new int(fib(num));
    return result;
}

void* payload2(void* arg) {
    int num = *static_cast<int*>(arg);
    int* result = new int(fib(num));
    return result;
}


double get_current_time()
{
  static int start = 0, startu = 0;
  struct timeval tval;
  double result;
  
  if (gettimeofday(&tval, NULL) == -1)
    result = -1.0;
  else if(!start) {
    start = tval.tv_sec;
    startu = tval.tv_usec;
    result = 0.0;
  }
  else
    result = (double) (tval.tv_sec - start) + 1.0e-6*(tval.tv_usec - startu);
  
  return result;
}

int main() {
    Pipeline pipeline;

    // Pipe pipe1(payload2);
    // Pipe pipe2(payload2);

    // Pipe pipe2(payload2);
    Farm farm1(8, payload1);
    Farm farm2(8, payload2);

    // pipeline.addStage(&pipe1);
    pipeline.addStage(&farm1);
    // pipeline.addStage(&pipe2);


    // pipeline.addStage(&farm1);
    pipeline.addStage(&farm2);

    ThreadSafeQueue<Task> inputQueue;
    for (int i = 0; i < 20; i++) {
        int* taskData = new int(900090000);
        inputQueue.enqueue(Task(taskData));
    }

    double beginning = get_current_time();

    ThreadSafeQueue<Result> output = pipeline.execute(inputQueue);

    double end = get_current_time();

    while (!output.empty()) {
        Result result;
        if (output.dequeue(result)) {
            int* res2 = static_cast<int*>(result.data);
            printf("%d\n", *res2);
            delete res2;
        }
    }

    std::cout << "Runtime is " << end - beginning << std::endl;

    return 0;
}