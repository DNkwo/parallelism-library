#include "para-pat/include/Farm.hpp"
#include "para-pat/include/Pipeline.hpp"
#include "para-pat/include/Pipe.hpp"
#include <sys/time.h>


//This is a test class that creates an unnecessarly complex parallel pattern to test robustness.

//function to get the current time in seconds
double get_current_time() {
  static int start = 0, startu = 0;
  struct timeval tval;
  double result;

  if (gettimeofday(&tval, NULL) == -1) {
    result = -1.0;
  } else if (!start) {
    start = tval.tv_sec;
    startu = tval.tv_usec;
    result = 0.0;
  } else {
    result = (double)(tval.tv_sec - start) + 1.0e-6 * (tval.tv_usec - startu);
  }
    
  return result;
}

//fibonacci function
int fib(int n)
{
  int  i, Fnew, Fold, temp,ans;

    Fnew = 1;  Fold = 0;
    for ( i = 2;
	  i <= n;          /* apsim_loop 1 0 */
	  i++ )
    {
      temp = Fnew;
      Fnew = Fnew + Fold;
      Fold = temp;
    }
    ans = Fnew;
  return ans;
}

//worker function
void* worker(void* arg) {
    int* n = static_cast<int*>(arg);
    int* result = new int;
    *result = fib(*n);
    
    return result;
}


int main() {

    int numtasks = 5; //number of tasks

    //creating the pipeline
    Pipeline pipeline;


    //creating the stages (might take 10-30 seconds as this nested pattern is complex)
    Pipe pipe(worker);
    Farm farm(1, worker);
    Farm farm2(3, worker);
    Farm farm3(2, worker);
    Farm farm4(3, worker);
    Pipe pipe1(worker);


    //adding the stages to the pipeline
    pipeline.addStage(&pipe);
    pipeline.addStage(&farm);
    pipeline.addStage(&farm2);
    pipeline.addStage(&farm3);
    pipeline.addStage(&farm4);

    //the output is probably going to be '1' because of integer overflow, fib(negative number) = 1

    //creating the input queue
    ThreadSafeQueue<Task> inputQueue;
    for (int i = 0; i < numtasks; i++) {
        int* taskData = new int(900090000); //any arbritary value
        inputQueue.enqueue(Task(taskData));
    }

    double beginning = get_current_time();

    // pipeline execution
    ThreadSafeQueue<Result> output = pipeline.execute(inputQueue);
    //terminating the pipleine
    pipeline.terminate();

    double end = get_current_time();

    //handling output (conversion from void* back to original datatype)
    while (!output.empty()) {
      Result result;
      if (output.dequeue(result)) {
          int* rs = static_cast<int*>(result.data);
          std::cout << "Result: " << *rs << std::endl;//the output is probably going to be '1' because of integer overflow
          delete rs; //prevent memory leaks
      }
    }


    printf("Runtime is %f\n", end - beginning);

    return 0;
}