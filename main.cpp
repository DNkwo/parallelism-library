#include "include/Farm.hpp"
#include "include/Pipeline.hpp"
#include "include/Pipe.hpp"


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

    //Preperation of tasks
    ThreadSafeQueue<Task> input;
    int n = 10;
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));
    input.enqueue(Task(&n));

    Pipeline pipeline;

    // Pipe pipe(worker);
    // Pipe pipe1(worker);
    // Pipe pipe2(worker);
    // Pipe pipe3(worker);

    Farm farm(3, worker);
    // Farm farm2(1, worker);

    // pipeline.addStage(&pipe);
    // pipeline.addStage(&pipe1);
    // pipeline.addStage(&pipe2);
    // pipeline.addStage(&pipe3);

    pipeline.addStage(&farm);

    ThreadSafeQueue<Result> output = pipeline.execute(input);
    pipeline.terminate();

    while (!output.empty()) {
        Result result;
        output.dequeue(result); //might be better to have a puttask and gettask function instead of enqueue and dequeue
        int* rs = static_cast<int*>(result.data);
        std::cout << "Result: " << *rs << std::endl;
    }
    
    std::cout << "hello" << std::endl;

    return 0;
}