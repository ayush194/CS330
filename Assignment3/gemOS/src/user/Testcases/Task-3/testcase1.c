#include<ulib.h>

/*simple cfork testcase */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  long * va = (long *)0x180001388;
  expand(100,MAP_WR);
  *va = 10;
  printf("Main number:%d\n", *va);
  pid = cfork();
  if(pid){
    long pid = getpid();
    printf("Parent pid:%u\n",pid);
    *va = 100;
    printf("Parent number:%d\n", *va);
  }
  else{
      long pid = getpid();
      printf("Child pid:%u\n",pid);
      printf("Child number:%d\n", *va);
  }
  get_stats();
  return 0;
}

