#include<ulib.h>

/*simple vfork testcase */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
 
  pid = vfork();
  printf("This should be printed twice\n");
  if(pid){
    long pid = getpid();
    printf("Parent pid:%u\n",pid);
  }
  else{
      long pid = getpid();
      printf("Child pid:%u\n",pid);
      sleep(5000);
      exit(0);
  }
  return 0;
}

