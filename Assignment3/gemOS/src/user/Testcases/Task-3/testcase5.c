#include<ulib.h>

/*mmap and call vfork testcase. Parent should not be able to print mm1[0]
since child has called munmap*/


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
  mm1[0] = 'A';
  pid = vfork();
  if(pid){
    printf("Parent mm1[0]:%c\n",mm1[0]);
  }
  else{
      mm1[0] = 'B';
      printf("Child mm1[0]:%c\n",mm1[0]);
      int val1 =  munmap(mm1, pages*2);
      if(val1 < 0)
      {
        printf("Map failed\n");
        return 1;
        }
      exit(0);
  }

  return 0;
}

