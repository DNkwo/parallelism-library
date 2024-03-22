// gcc example.c -o example


#include <stdio.h>

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
        int i;
	for (i = 0; i < 100; i++) {
		printf("%d\n", i);
		int res = payload1(i);
		int res2 = payload2(res);
	}

}
