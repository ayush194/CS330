#include<ulib.h>

void* util_mmap(void * addr, int length, int prot, int flag)
{
  char* temp = mmap(addr, length, prot, 0);
  if((long)temp <= 0)
  {
    printf("Testcase Failed \n");
    exit(0);
  }
  return temp;
}

int util_munmap(void * addr, int length)
{
   int temp = munmap(addr, length);
   if(temp < 0)
   {
     printf("Testcase Failed \n");
     exit(0);
   }
   return temp;
}

int util_mprotect(void * addr, int length, int prot)
{

  int temp = mprotect(addr, length, prot);
  if(temp < 0)
  {
    printf("Testcase Failed \n");
    exit(0);
  }
  return temp;
}

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  
 
  int total = 63;

  // Creating Bulk pages;
  char * mm1= util_mmap(NULL, (pages*100*total), PROT_WRITE, 0);
  // vm_area count should be 1
  pmap(0);
  
  // Changing PROTECTION for every 100 alternate pages 
  for(int i =0; i < total; i++)
  {
      int prot = i % 2 == 0? PROT_WRITE : PROT_READ;
      char* vm_area_start = mm1 + (i * (pages*100));
      util_mprotect(vm_area_start, pages*100, prot);
  }
  // Vm_area count should be  63;
   pmap(0);

  // Changing protection 10 PAGES FROM FRONT. So that End of the previous vm_area end will be expanded
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100));
      //unmapping 10 pages from front
      util_mprotect(vm_area_start, pages*10, PROT_WRITE);
  }
  // Vm_area count should be  63;
  pmap(0);


  // Changing protection 10 PAGES FROM Back. So that End of the next vm_area start will be expanded
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100));
      char* vm_area_back_10_pages = vm_area_start + (pages*90);
      util_mprotect(vm_area_back_10_pages, pages*10, PROT_WRITE);
  }
  // Vm_area count should be  63;
  pmap(0);


  // Changing protection in the center of vm_area, so that vm_Area will be splitted
  for(int i =1; i < total; i = i + 2)
  {
        char* vm_area_start = mm1 + (i * (pages*100)) + pages*20;
        util_mprotect(vm_area_start, pages*10, PROT_WRITE);
  }
  // Vm_area count should be  125;
  pmap(0);


  // Will merge all the vm_area (PROT_READ) which was splitted earlier.
  for(int i =1; i < total;  i=i+2)
  {
     char* vm_area_start = mm1 + (i * (pages*100)) + pages*20;
     util_mprotect(vm_area_start, pages*10, PROT_READ);
  }
  // Vm_area count should be  63;
  pmap(0);


  // Changing the PROT_READ to PROT_WRITE. So that all the vm_area will be merged to be one.
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100 ));
      char * read_area_begining = vm_area_start + (pages*10 );
      //unmapping 10 pages from front
      util_mprotect(read_area_begining, pages*80, PROT_WRITE);
  }
  // vm_area count should be 1
   pmap(0);


  util_munmap(mm1, (pages*total *100));

  //Vm_area count should be  0;
  pmap(0);

return 0;
}






/*simple cfork testcase */
/*
//task 3 testcase 1
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  long * va = (long *)0x180001388;
  expand(100,MAP_WR);
  pmap(1);
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
*/

/*simple vfork testcase */
/*
//task 3 testcase 2
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
*/

/*mmap and call cfork testcase. Parent should be able to print mm1[0]
even after child called munmap */
/*
//task 3 testcase 3
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
  pid = cfork();
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
    printf("Parent mm1[0]:%c\n",mm1[0]);

  return 0;
}
*/

/*mmap and call cfork testcase. Parent should be able to print mm1[0]
even after child called munmap */
/*
//task 3 testcase 4
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
  pid = cfork();
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
    printf("Parent mm1[0]:%c\n",mm1[0]);

  return 0;
}
*/

/*mmap and call vfork testcase. Parent should not be able to print mm1[0]
since child has called munmap*/
/*
//task 3 testcase 5
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
*/


/*parent mmap and call cfork, then writes to mmaped area, copy-on-write should happen */
/*
//task 3 testcase 6
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, MAP_POPULATE);
  if(mm1 < 0)
  {
    printf("Map failed \n");
    return 1;
  }
  mm1[0] = 'A';
  mm1[1] = 'A';
  pmap(1);
  pid = cfork();
  if(pid){
    printf("Parent mm1[0]:%c\n",mm1[0]);
    mm1[1] = 'B';
    printf("Parent mm1[1]:%c\n",mm1[1]);
  }
  else{
      printf("Child mm1[0]:%c\n",mm1[0]);
      printf("Child mm1[1]:%c\n",mm1[1]);
  }
  int val1 = munmap(mm1, pages*2);
  if(val1 < 0)
  {
    printf("Map failed\n");
    return 1;
  }
  //get_stats();
  return 0;
}
*/

//task 3 testcase 7
/*parent mmap and call cfork,
then parent and child writes.
first write causes copy-on-write and increase user_region_pages stats
but second write only changes PTE entry */
/*
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
  mm1[1] = 'A';
  pmap(1);
  pid = cfork();
  if(pid){
    
    printf("Parent mm1[0]:%c\n",mm1[0]);
    mm1[1] = 'B';
    printf("Parent mm1[1]:%c\n",mm1[1]);
  }
  else{
      printf("Child mm1[0]:%c\n",mm1[0]);
      mm1[1] = 'B';
      printf("Child mm1[1]:%c\n",mm1[1]);
  }
  int val1 = munmap(mm1, pages*2);
  if(val1 < 0)
  {
    printf("Map failed\n");
    return 1;
  }
  get_stats();
  return 0;
}
*/

//task 3 testcase 8
/*parent mmap and call vfork, then child writes to mmaped area, parent should see the modification done by child to mmaped area */
/*
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
*/

//task 3 testcase 9
/*dump page table to verify the PTE entry */
/*
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
*/


/*
//task 2 testcase 1
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
  pmap(1);
  
  for(int i = 0; i<50; i++)
  {
    addr_map_populate[(pages * i)] = 'X';
  }
  // Number of MMAP_Page_Faults should be 0 & 
  // Number of vm_area should 1
  pmap(1);


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
  pmap(1);
 return 0;
}
*/

/*
//task 2 testcase 2
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;
  // vm_area will be created without physical pages.
  char * lazy_alloc = mmap(NULL, pages*50, PROT_READ|PROT_WRITE, 0);
  if((long)lazy_alloc < 0)
  {
    // Testcase failed.
    printf("Test case failed \n");
    return 1;
  }
  pmap(1);

  // All access should result in page fault.
  for(int i = 0; i<50; i++)
  {
    lazy_alloc[(pages * i)] = 'X';
  }
  // Number of MMAP_Page_Faults should be 50 & 
  // Number of vm_area should 1
  pmap(0);

  for(int i = 0; i<50; i++)
  {
    // Reading the value from physical page. It should be same as written
    if(lazy_alloc[(pages * i)] != 'X')
    {
      // Testcase Failed;
      printf("Test case failed \n");
      return 0;
    }
  }
  // Number of MMAP_Page_Faults should be 50 & 
  // Number of vm_area should 1
  pmap(0);

 return 0;
}
*/

/*
//task 2 testcase 3
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;
  char * mm1 = mmap(NULL, pages*6, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
 char * readonly_address = (char *)((unsigned long)mm1 + pages* 3);
 readonly_address[0] = 'X'; 
  // vm_area count should be 1 and Page fault should be 1
  pmap(1);

  if(readonly_address[0] != 'X')
  {
    // Testcase failed
     printf("Test case failed \n");
    return 0;
  }


  // Should change access rights of the third page, existing vm_Area should be splitted up. 
  // A new vm area with access rights with PROT_READ will be created.
  int result  = mprotect((void *)readonly_address, pages, PROT_READ);

  if(result <0)
  {
    // Testcase failed
     printf("Test case failed \n");
    return 0;
  }
  // vm_area count should be 3.
  pmap(1);


  // Access violation, Writing to a read only address. Prpcess should be terminate while executing the below line
  readonly_address[0] = 'X';

  printf("Test case failed \n");

 return 0;
}
*/

/*
//testcase 1
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  
  char *addr1 = mmap(NULL, 22, PROT_READ|PROT_WRITE, 0);
  if((long)addr1 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }
  // Vm_Area count should be 1
  // Expected output will have address printed. In your case address printed might be different.
  // But See the printed address, (i.e) the start and the end address of the dumped vm area is page aligned irrespective of the length provided.
  pmap(1);


  // Access flag is different should create a new vm_area
  char *addr2 = mmap(NULL, 4096, PROT_WRITE, 0);
  if((long)addr2 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }
  //  Vm_Area count should be 2
  pmap(1);


  int munmap1 = munmap(addr2, 4096);

  if(munmap1 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }
   // Vm_Area count should be 1
  pmap(0);

  

  int munmap2 = munmap(addr1, 22);

  if(munmap2 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }
  // Vm_Area count should be 0
  pmap(0);
  return 0;
}
*/

/*
//testcase 2
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

  int pages = 4096;

  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if((long) mm1 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1
  pmap(1);

  // The below mmap request will be merged with the existing vm_area as the access flags are same as previous vm_area
  // mm2 address should be the next immediate address of (mm1 + pages*2);
  char * mm2 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);

 
  char * check_addr = mm1 + (pages*2);
  
  if((long)mm2 < 0 &&  (long)mm2 != (long)check_addr)
  {
    printf("Test case failed \n");
    return 1;
  }

  // vm_area count should be 1, It will be merged with existing area
  pmap(1);

  char * mm3 = mmap(NULL, pages*4, PROT_READ, 0);
  if((long)mm3 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }

  // New vm_area should be created as access rights are different.
  // vn_area count should be 2
  pmap(1);

  int val1 = munmap(mm3, pages*2);
  if(val1 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }

  // vm_area count should be 2. 
  pmap(1);

  char * mm4 = mmap(NULL, pages*2, PROT_READ, 0);
    if((long)mm4 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }

  // vm_area count should be 2
  pmap(1);

  int val = munmap(mm4 + pages , pages*2);
  if(val < 0)
  {
    printf("Test case failed \n");
    return 1;
  }

  // Area unmapped is inside the vm_area. So vm_area will be splitted into two.
  // vm_area count should be 3
  pmap(1);

  return 0;
}
*/

/*
//testcase 3
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  char * mm1 = mmap(NULL, pages*50, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1
  pmap(0);

  for(int i = 0; i < 50; i++)
  {
      if(i % 2 != 0)
      {
          unsigned long addr = (unsigned long)mm1 + (pages * i);
          int val1 = munmap((void*)addr, pages);
          if(val1 < 0)
          {
            printf("Test case failed \n");
            return 1;
          }
      }
  }
  // vm_area count should be 25
  pmap(0);

  for(int i = 0; i < 50; i++)
  {
      if(i % 2 != 0)
      {
          char * mm2 = mmap(NULL, pages, PROT_READ|PROT_WRITE, 0);
          if((long)mm2 < 0)
          {
            printf("Test case failed \n");
            return 1;
          }
      }
  }
  // vm_area count should be 1;  
  pmap(0);

return 0;
}
*/

/*
//testcase 4
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 <= 0)
  {
    // Testcase failed.
    printf("Test case failed \n");
    return 1;
  }
    // vm_area count should be 1.
  pmap(1);


  // Hint address is free (not mapped). So it should create a vm_area with requested hint address and return it.
  char *hint_address = mm1 + (pages * 10);

  char *mm2  = mmap(hint_address, pages,PROT_READ|PROT_WRITE, 0);

  if(mm2 <= 0 || (long)hint_address != (long)mm2)
  {
    // Testcase failed
    printf("Test case failed \n");
    pmap(1);
    return 0;
  }
    // vm_area count should be 2.
  pmap(1);

  // Hint address is already mapped.  It should look 
  // for the next immediated free addresses in the address space 
  // which can server the request and return it successfully.
  char *mm3  = mmap(mm1, pages,PROT_READ, 0);

  if(mm3 <= 0)
  {
    // Testcase failed
    printf("Test case failed \n");
    return 0;
  }
  
  // vm_area count should be 3.
  pmap(1);


  int munmap1 = munmap(mm3, pages);

  if(munmap1 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }

   // vm_area count should be 2.
  pmap(1);


  int munmap2 = munmap(mm2, pages);

  if(munmap2 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }

   // vm_area count should be 1.
  pmap(1);


  int munmap3 = munmap(mm1, pages*2);

  if(munmap3 < 0)
  {
    printf("TEST CASE FAILED\n");
    return 1;
  }

   // vm_area count should be 0.
  pmap(1);

 return 0;
}
*/

/*
//testcase 5
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

  int pages = 4096;

  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    // Testcase failed.
    printf("Test case failed \n");
    return 1;
  }
  pmap(1);


  char *hint_address = mm1 + (pages * 10);
// Hint address is free (not mapped). So it should create a vm_area with requested hint address and return it.
  char *mm2  = mmap(hint_address, pages,PROT_READ|PROT_WRITE, MAP_FIXED);

  if((long)mm2 < 0 || (long)hint_address != (long)mm2)
  {
    // Testcase failed
    printf("Test case failed \n");
    pmap(1);
    return 0;
  }

  // vm_area count should be 2.
  pmap(1); 

  // Hint address is already mapped with some vm_area. So mmap should fail and return-1 because of MAP_fixed flag
  char *mm3  = mmap(mm1, pages,PROT_READ, MAP_FIXED);
  if((long)mm3 > 0)
  {
     // Testcase failed.
     printf("%x Test case failed %x \n", mm3, mm1);
     pmap(1);
     return 0;
  }
  // vm_area count should be 2.
  pmap(1); 
 return 0;
}
*/

/*
//testcase 6
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

  int pages = 4096;

  char * mm1 = mmap(NULL, pages*6, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    // Testcase failed.
     printf("Test case failed \n");
    return 1;
  }
  // vm_area count should be 1.
  pmap(1);

  unsigned long readonly_address = (unsigned long)mm1 + pages* 3;

  // Should change access rights of the third page, existing vm_Area should be splitted up. A new vm area with access rights with PROT_READ will be created.
  int result  = mprotect((void *)readonly_address, pages, PROT_READ);

  if(result <0)
  {
    // Testcase failed
     printf("Test case failed \n");
    return 0;
  }
  
  // vm_area count should be 3.
  pmap(1);

  unsigned long invalid_Address =  (unsigned long)mm1 + pages* 50;

  // mprotect should fail.
  int result1  = mprotect((void *)invalid_Address, pages,PROT_READ);

  if(!(result1 <0))
  {
    // Testcase failed
     printf("Test case failed \n");
    return 0;
  }

  //vm_area count should be 3
  pmap(1); 
 return 0;
}
*/

/*
//custom testcase
char* x = NULL;
void swap(int *xp, int *yp)  
{  
    int temp = *xp;  
    *xp = *yp;  
    *yp = temp;  
}  
void merge(int* arr, int l, int m, int r) 
{ 
    int i, j, k; 
    int pages = 4096;
    int n1 = m - l + 1; 
    int n2 =  r - m; 
    // create temp arrays
    char* L1 = mmap(0x180242000 + 30*pages ,4*n1,PROT_WRITE | PROT_READ,0);
    int* L = L1;
    int R[n2];
    // int L[n1];
    // Copy data to temp arrays L[] and R[]
    for (i = 0; i < n1; i++) 
        L[i] = arr[l + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = arr[m + 1+ j]; 
  
    // Merge the temp arrays back into arr[l..r]
    i = 0; // Initial index of first subarray 
    j = 0; // Initial index of second subarray 
    k = l; // Initial index of merged subarray 
    mprotect(L1,4*n1,PROT_READ);
    while (i < n1 && j < n2) 
    { 
        if (L[i] <= R[j]) 
        { 
            arr[k] = L[i]; 
            i++; 
        } 
        else
        { 
            arr[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
  
    // Copy the remaining elements of L[], if there are any
    while (i < n1) 
    { 
        arr[k] = L[i]; 
        i++; 
        k++; 
    } 
  
    // Copy the remaining elements of R[], if thereare any
    while (j < n2) 
    { 
        arr[k] = R[j]; 
        j++; 
        k++; 
    } 
    munmap(L1,4*n1);
} 

void mergeSort(int* arr, int l, int r) 
{ 
    if (l < r) 
    { 
        // Same as (l+r)/2, but avoids overflow for 
        // large l and h 
        int m = l+(r-l)/2; 
  
        // Sort first and second halves 
        mergeSort(arr, l, m); 
        mergeSort(arr, m+1, r); 
  
        merge(arr, l, m, r); 
    } 
} 

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
    int a = 1;
    int n = 446,cycles = 900;
    u64 pages = 4096;
  int* mm;
  // Check if sorting works in stack
  printf("Checking stack correctly working...\n");
  int arr[n];
  int i, j;  
  for(int i=0; i<n ; i++) arr[i] = n-1-i;
  for (i = 0; i < n-1; i++)      
    // Last i elements are already in place  
    for (j = 0; j < n-i-1; j++)  
        if (arr[j] > arr[j+1])  
            swap(&arr[j], &arr[j+1]);  
  for(int i=0; i<n; i++) if(arr[i] != i) a = 0;
  if(a) printf("stack correctly working\n");
  else printf("stack not working\n");
  //===============================
  cycles = 900;
  while(cycles--){
    for(int i = 0; i<n; i++) arr[i] = i;
  }
  //===============================
  printf("Now will check if mmap working correctly");
  char* mm1 = mmap(NULL, pages*6, PROT_READ|PROT_WRITE, 0);
 // x = mm1;
  // printf("%x == %x?\n",x,mm1);
  for(int i=3; i<129; i++){
    if(i%2) mmap(mm1 + 2*pages*i, pages*2,PROT_READ, MAP_POPULATE);
    else mmap(mm1 + 2*pages*i, pages*2,PROT_WRITE, 0);
  }
  pmap(1);
  printf("Verify that 31 regions are found, 1st has RW, others alternately R and W\n");
  //===============================
  cycles = 900;
  while(cycles--){
    for(int i = 0; i<n; i++) arr[i] = i;
  }
  //===============================
  mprotect(mm1 + 7*pages, 11*pages, PROT_READ | PROT_WRITE);
  munmap(mm1 + 21*pages, 4*pages + 343);
  printf("Checking if sorting is working...\n");
   mm = mm1 + 7*pages;
  for(int i=0; i<n ; i++) mm[i] = n-1-i;
  for (i = 0; i < n-1; i++)      
    // Last i elements are already in place  
    for (j = 0; j < n-i-1; j++)  
        if (mm[j] > mm[j+1])  
            swap(&mm[j], &mm[j+1]);   
  for(int i=0; i<n; i++) if(mm[i] != i) a = 0;
  if(a) printf("Sorting correctly working\n");
  else printf("Sorting not working\n");
    //===============================
  cycles = 900;
  while(cycles--){
    for(int i = 0; i<n; i++) mm[i] = i;
  }
  //===============================
  pmap(1);
  // printf("PASSED++++++++++++++++++++\n");
  printf("Verify that 25 regions are found\n");
  //===============================
  cycles = 900;
  while(cycles--){
    for(int i = 0; i<n; i++) mm[i] = i;
  }
  //===============================
  mprotect(mm1 + 28*pages, 8*pages, PROT_READ);
  mprotect(mm1 + 43*pages, 5*pages, PROT_READ | PROT_WRITE);
  munmap(mm1 + 24*pages, 14*pages + 343);
  printf("Checking if sorting is working...\n");
  mm = mm1 + 43*pages + 4088;
  for(int i=0; i<n ; i++) mm[i] = n-1-i;
  for (i = 0; i < n-1; i++)      
    // Last i elements are already in place  
    for (j = 0; j < n-i-1; j++)  
        if (mm[j] > mm[j+1])  
            swap(&mm[j], &mm[j+1]);
  // for(int i=0; i<n; i++) printf("%d\n",mm[i]);
  for(int i=0; i<n; i++) if(mm[i] != i) a = 0;
  if(a) printf("Soring correctly working\n");
  else printf("Sorting not working\n");
  //===============================
  cycles = 900;
  while(cycles--){
    for(int i = 0; i<n; i++) mm[i] = i;
  }
  //===============================
  pmap(1);
  printf("number of vm_areas should be 18 and page_fault = 1\n");
  //===============================
  cycles = 900;
  while(cycles--){
    for(int i = 0; i<n; i++) mm[i] = i;
  }
  // ===============================
  printf("PASSED++++++++++++++++++++\n");
  printf("Now will apply merge sort on an array!\n");
  //===============================
  cycles = 900;
  while(cycles--){
    for(int i = 0; i<n; i++) mm[i] = i;
  }
  //===============================
  pmap(1);
  u32 pid = cfork();
  if(pid){
    u32 pid2 = vfork();
    if(!pid2){
        n = 3437;
        mm = mmap(NULL,4*n,PROT_READ|PROT_WRITE,0);
        printf("Successful mmap\n");
        for(int i=0; i<n ; i++) mm[i] = n-1-i;
        // printf("OK");
        mergeSort(mm,0,n-1);
        // for(int i=0; i<n; i++) printf("%d\n",mm[i]);
        for(int i=0; i<n; i++) if(mm[i] != i) a = 0;
        if(a) printf("Soring correctly working\n");
        else printf("Sorting not working\n");
        exit(0);
    }
    else{
        printf("The child vfork() has run correctly\n");
    }
    munmap(mm,4*n);
    get_stats();
    printf("Parent exiting \n");
  }
  else{
  //===============================
  cycles = 10;
  while(cycles--){
    for(int i = 0; i<n; i++) mm[i] = i;
  }
  //=============================== 
    printf("Child waiting from cfork() is over\n");
    printf("Lets see if the child can destroy the ambitions of vfork() child");
    munmap(mm,4*n);
    get_stats();
    printf("cfork() child running over");
  }
  return 0;
}
*/

/*
//custom testcase
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

  int pages = 4096;

  char * mm1 = mmap(NULL, pages*10, PROT_READ, 0);
  char * mm2 = mmap(NULL, pages*10, PROT_WRITE, 0);
  pmap(1);
  mprotect((void *)mm1 + 8*pages, 2*pages, PROT_WRITE);
  pmap(1);
  mprotect((void *)mm1 + 8*pages, 2*pages, PROT_READ);
  pmap(1);
  mprotect((void *)mm1 + 15*pages, 2*pages, PROT_READ | PROT_WRITE);
  pmap(1);
  mprotect((void *)mm1 + 11*pages, 2*pages, PROT_READ | PROT_WRITE);
  pmap(1);
  mprotect((void *)mm1 + 13*pages, 2*pages, PROT_READ | PROT_WRITE);
  pmap(1);
 return 0;
}
*/

/*
//custom testcase
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{

  int pages = 4096;
  for(int i = 0; i < 65; i++) {
  	char* mm1 = mmap(NULL, pages, PROT_READ, 0);
  	if ((long)mm1 < 0) printf("mmap failed!\n");
  	//printf("%c", mm1);
  	//if (i == 64) pmap(1);
  	pmap(0);
  	char* mm2 = mmap(NULL, pages, PROT_WRITE, 0);
  	if ((long)mm2 < 0) printf("mmap failed!\n");
  	//printf("%c", mm2);
  	//if (i == 64) pmap(1);
  	pmap(0);
  }
 return 0;
}
*/