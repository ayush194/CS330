#include<ulib.h>

/*dump page table to verify the PTE entry */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  long * va = (long *)0x180001388;
  expand(100,MAP_WR);
  *va = 10;
  dump_page_table((char*)va);
  pid = cfork();
  if(pid){
    long pid = getpid();
    printf("Parent pid:%u\n",pid);
    dump_page_table((char*)va);
  }
  else{
      long pid = getpid();
      printf("Child pid:%u\n",pid);
      dump_page_table((char*)va);
  }
  return 0;
}

