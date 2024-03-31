#include "para-pat/include/Farm.hpp"
#include "para-pat/include/Pipeline.hpp"
#include "para-pat/include/Pipe.hpp"
#include "para-pat/include/StageManager.hpp"


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


void* worker(void* arg) {
    int* n = static_cast<int*>(arg);
    int* result = new int;
    *result = fib(*n);
    
    return result;
}


int main() {

    Pipeline pipeline;

    Pipe pipe(worker);
    Farm farm(1, worker);
    Farm farm2(3, worker);
    Farm farm3(2, worker);
    Farm farm4(3, worker);
    Pipe pipe1(worker);

    pipeline.addStage(&pipe);
    pipeline.addStage(&farm);
    pipeline.addStage(&farm2);
    pipeline.addStage(&farm3);
    pipeline.addStage(&farm4);


    ThreadSafeQueue<Task> inputQueue;
    for (int i = 0; i < 2; i++) {
        int* taskData = new int(5);
        inputQueue.enqueue(Task(taskData));
    }

    ThreadSafeQueue<Result> output = pipeline.execute(inputQueue);
    pipeline.terminate();

    std::cout << output.size() << std::endl;

    while (!output.empty()) {
      Result result;
      if (output.dequeue(result)) {
          int* rs = static_cast<int*>(result.data);
          std::cout << "Result: " << *rs << std::endl;
          delete rs; //prevent memory leaks
      }
    }
    
    std::cout << "dhdd" << std::endl;


    return 0;
}