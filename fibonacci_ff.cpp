// g++ -I/home/chaysse/fastflow -o fibonacci_ff fibonacci_ff.cpp -pthread

#include <vector>
#include <iostream>
#include <ff/ff.hpp>
#include <sys/time.h>

using namespace ff;

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

//fib function
int fib(int n) {
  int Fnew = 1, Fold = 0, temp;

  for (int i = 2; i <= n; i++) {
    temp = Fnew;
    Fnew = Fnew + Fold;
    Fold = temp;
  }

  return Fnew;
}

//worker class for payload 1
class Payload1Worker : public ff_node {
public:
  void* svc(void* arg) {
    int num = *static_cast<int*>(arg);
    int* result = new int(fib(900090000));
    return result;
  }
};

//worker class for payload 2
class Payload2Worker : public ff_node {
public:
  void* svc(void* arg) {
    int num = *static_cast<int*>(arg);
    int* result = new int(fib(900090000));
    return result;
  }
};

//emits the tasks
class Emitter : public ff_node {
public:
  Emitter(int max_task) : ntask(max_task) {}

  void* svc(void*) {
    if (ntask < 0) return nullptr;
    int* task = new int(ntask);
    --ntask;
    return task;
  }

private:
  int ntask;
};

int main() {


  int nworkers = 3; //number of workers
  int numtasks = 20; //number of tasks

  ff_pipeline pipeline; //creates pipeline

  // the constructor is (custom emitter, max num of workers, min number of workers, collector, input queue size, on demand)
  ff_farm farm1(false, nworkers, nworkers, false, 32, true);


  Emitter E(numtasks); //emits tasks


  //the code below aims to create a Farm(N) -> Farm(N) pattern, this was the last one I was using when writing report

  farm1.add_emitter(&E);
  std::vector<ff_node*> w1;
  for (int i = 0; i < nworkers; ++i)
    w1.push_back(new Payload1Worker);
  farm1.add_workers(w1);
  pipeline.add_stage(&farm1);

  // Create the second farm with 4 workers
  ff_farm farm2(false, nworkers, nworkers, false, 32, true);
  std::vector<ff_node*> w2;
  for (int i = 0; i < nworkers; ++i)
    w2.push_back(new Payload2Worker);
  farm2.add_workers(w2);
  pipeline.add_stage(&farm2);


  //timing
  double beginning = get_current_time();

  pipeline.run_and_wait_end();

  double end = get_current_time();

  printf("Runtime is %f\n", end - beginning);

  return 0;
}