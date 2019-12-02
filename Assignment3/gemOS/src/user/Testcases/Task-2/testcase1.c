#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  //Physical pages will be mapped for all the 50 pages.
  char * addr_map_populate = mmap(NULL, pages*50, PROT_READ|PROT_WRITE, MAP_POPULATE);
  if((long)addr_map_populate < 0)
  {
    // Testcase failed.
    printf("Test case failed \n");
    return 1;
  }
  
  for(int i = 0; i<50; i++)
  {
    addr_map_populate[(pages * i)] = 'X';
  }
  // Number of MMAP_Page_Faults should be 0 & 
  // Number of vm_area should 1
  pmap(0);


  for(int i = 0; i<50; i++)
  {
    // Reading the value from physical page. It should be same as written
    if(addr_map_populate[(pages * i)] != 'X')
    {
      // Testcase Failed;
      printf("Test case failed \n");
      return 0;
    }
  }

  // Number of MMAP_Page_Faults should be 0 & 
  // Number of vm_area should 1
  pmap(0);
 return 0;
}
