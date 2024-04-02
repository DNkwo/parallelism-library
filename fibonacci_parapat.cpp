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

    int numtasks = 20; //number of tasks


    //generating tasks and putting them into input queue
    ThreadSafeQueue<Task> inputQueue;
    for (int i = 0; i < numtasks; i++) {
        int* taskData = new int(900090000);
        inputQueue.enqueue(Task(taskData));
    }

    //create pipeline
    Pipeline pipeline;

    //creating stages
    Pipe pipe(payload1);
    Farm farm(8, payload2);

    //adding stages to pipeline
    pipeline.addStage(&pipe);
    pipeline.addStage(&farm);

    //measuring time
    double beginning = get_current_time();


    //executing pipeline
    ThreadSafeQueue<Result> output = pipeline.execute(inputQueue);

    double end = get_current_time();


    //processing results (conversion from void* back to original data point)
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