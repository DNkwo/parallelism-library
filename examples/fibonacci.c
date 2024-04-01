// gcc example.c -o example

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

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


int payload1(int i)
{
   return (fib(900090000)); 
}

int payload2(int j)
{
  return (fib(900090000));
}




int main ()
{
  double beginning = get_current_time();
        int i;
	for (i = 0; i < 20; i++) {
		printf("%d\n", i);
		int res = payload1(i);
		int res2 = payload2(res);
	}

  double end = get_current_time();

  printf("Runtime is %f\n", end - beginning);

}
