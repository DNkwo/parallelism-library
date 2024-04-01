// g++ fibonacci_ff.cpp -o fibonacci_ff -pthread -I/path/to/fastflow

#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

using namespace ff;

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

struct Payload1 : ff_node_t<int, int> {
  int svc(int i) {
    return fib(900090000);
  }
};

struct Payload2 : ff_node_t<int, int> {
  int svc(int j) {
    return fib(900090000);
  }
};

struct Emitter : ff_node_t<int> {
  int svc(int) {
    for (int i = 0; i < 20; i++) {
      ff_send_out(i);
    }
    return EOS;
  }
};

struct Collector : ff_node_t<int> {
  int svc(int result) {
    printf("%d\n", result);
    return GO_ON;
  }
};

int main() {
  double beginning = get_current_time();

  ff_Farm<> farm1(true, 4);
  farm1.add_emitter(new Emitter());
  farm1.add_collector(new Collector());
  std::vector<ff_node*> workers1;
  for (int i = 0; i < 4; ++i) {
    workers1.push_back(new Payload1());
  }
  farm1.add_workers(workers1);

  ff_Farm<> farm2(true, 4);
  std::vector<ff_node*> workers2;
  for (int i = 0; i < 4; ++i) {
    workers2.push_back(new Payload2());
  }
  farm2.add_workers(workers2);

  ff_Pipe<> pipe(farm1, farm2);
  pipe.run_and_wait_end();

  double end = get_current_time();

  printf("Runtime is %f\n", end - beginning);

  return 0;
}