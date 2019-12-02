#include<ulib.h>

/*parent mmap and call vfork, then child writes to mmaped area, parent should see the modification done by child to mmaped area */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if(mm1 < 0)
  {
    printf("Map failed \n");
    return 1;
  }
  for(int i=0;i<10;i++){
    mm1[i] = 'A';
  }
  pid = vfork();
  if(pid){
    printf("Parent mm1[0]:%c\n",mm1[0]);
    printf("Parent mm1[1]:%c\n",mm1[1]);
  }
  else{
      printf("Child mm1[0]:%c\n",mm1[0]);
      mm1[1] = 'B';
      printf("Child mm1[1]:%c\n",mm1[1]);
      exit(0);
  }
  int val1 = munmap(mm1, pages*2);
  if(val1 < 0)
  {
    printf("Map failed\n");
    return 1;
  }
  return 0;
}

