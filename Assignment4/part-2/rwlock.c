/*
CS330 Assignment4 Part2

Name: Ayush Kumar
Roll No: 170195
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<common.h>

/* XXX NOTE XXX  
       Do not declare any static/global variables. Answers deviating from this 
       requirement will not be graded.
*/
void init_rwlock(rwlock_t *lock)
{
   /*Your code for lock initialization*/
   //initially unlocked for all operations
   lock->value = 0x1000000000000;
}

void write_lock(rwlock_t *lock)
{
   /*Your code to acquire write lock*/
   while(atomic_add(&lock->value, -0x1000000000000)) {
      atomic_add(&lock->value, 0x1000000000000);
      sched_yield();
   }
   //while (lock->value != 0x1000000000000);   //wait till there are 0 readers
   //lock->value = 0x0000000000000;
}

void write_unlock(rwlock_t *lock)
{
   /*Your code to release the write lock*/
   //lock->value = 0x1000000000000;
   atomic_add(&lock->value, 0x1000000000000);
}


void read_lock(rwlock_t *lock)
{
   /*Your code to acquire read lock*/
   //should not read if write lock is acquired
   //while(lock->value == 0x0000000000000);
   while(atomic_add(&lock->value, -1) < 0) {
      atomic_add(&lock->value, 1);
      sched_yield();
   }
}

void read_unlock(rwlock_t *lock)
{
   atomic_add(&lock->value, 1);
   /*Your code to release the read lock*/
}
