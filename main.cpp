#include "include/Farm.hpp"
#include "include/Pipeline.hpp"
#include "include/Pipe.hpp"
#include "include/StageManager.hpp"


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
    int* taskData;
    for (int i = 0; i < 8; ++i) {
        taskData = new int(3);
        input.enqueue(Task(taskData));
    }

    StageManager manager;

    Pipe pipe(worker);
    Farm farm(1, worker);
    Farm farm2(3, worker);
    Farm farm3(2, worker);
    Farm farm4(3, worker);
    Pipe pipe1(worker);

    manager.addStage(&pipe);
    manager.addStage(&farm);
    manager.addStage(&farm2);
    manager.addStage(&farm3);
    manager.addStage(&farm4);
    manager.addStage(&pipe1);

    ThreadSafeQueue<Result> output = manager.execute(input);
    manager.terminate();

    while (!output.empty()) {
        Result result;
        output.dequeue(result); //might be better to have a puttask and gettask function instead of enqueue and dequeue
        int* rs = static_cast<int*>(result.data);
        std::cout << "Result: " << *rs << std::endl;
    }
    
    std::cout << "hello" << std::endl;

    return 0;
}