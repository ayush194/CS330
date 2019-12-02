/*
CS330 Assignment4 Part3

Name: Ayush Kumar
Roll No: 170195
*/

#include "common.h"
//#include <stdio.h>

/*Function templates. TODO*/

static int atomic_add(long *ptr, long val) __attribute__((noinline));


/*
  Return value is as follows,
   0 ==> result is zero
   1 ==> result is positive
   -1 ==> result is negative 
*/
static int atomic_add(long *ptr, long val)
{
       int ret = 0;
       asm volatile( 
                     "lock add %%rsi, (%%rdi);"
                     "pushf;"
                     "pop %%rax;" 
                     "movl %%eax, %0;"
                     : "=r" (ret)
                     : 
                     : "memory", "rax"
                    );

     
     if(ret & 0x80)
               ret = -1;
     else if(ret & 0x40)
               ret = 0;
     else
               ret = 1;
     return ret;
}

void done_one(struct input_manager *in, int tnum) {
	pthread_mutex_lock(&in->lock);
	in->being_processed[tnum] = NULL;
	pthread_cond_broadcast(&in->cond);
	pthread_mutex_unlock(&in->lock);
}

int read_op(struct input_manager *in, op_t *op, int tnum) {
	pthread_mutex_lock(&in->lock);
	//other threads will not be allowed to change the value of in->curr;
	unsigned size = sizeof(op_t);
	memcpy(op, in->curr, size - sizeof(unsigned long));  //Copy till data ptr     
	if(op->op_type == GET || op->op_type == DEL){
		in->curr += size - sizeof(op->datalen) - sizeof(op->data);
	}else if(op->op_type == PUT){
		in->curr += size - sizeof(op->data);
		op->data = in->curr;
		in->curr += op->datalen;
	}else{
		//assert(0);
	}
	if(in->curr > in->data + in->size) {
		pthread_mutex_unlock(&in->lock);
		return -1;
	}
	in->being_processed[tnum] = op;
	pthread_mutex_unlock(&in->lock);

	//check if all operations for op->key before op->id before have finished
	for(int td_num = 0; td_num < THREADS; td_num++) {
		pthread_mutex_lock(&in->lock);
		if (td_num != tnum && in->being_processed[td_num] && in->being_processed[td_num]->key == op->key
			&& in->being_processed[td_num]->id < op->id) {
			//there is some other prior PUT/DEL being performed by thread td_num, so wait for it
			pthread_cond_wait(&in->cond, &in->lock);
			td_num = -1;
		}
		pthread_mutex_unlock(&in->lock);
	}
	//all operations before op->id have been performed, now we can allow this search/delete/insert/update to occur

	return 0; 
}

int lookup(hash_t *h, op_t *op) {
	//before performing a lookup, ensure all operations before it have finished
	unsigned ctr;
	unsigned hashval = hashfunc(op->key, h->table_size);
	hash_entry_t *entry = h->table + hashval;
	ctr = hashval;
	//searches for the first entry which is either empty or contains the same key
	//loops as long as it sees filled (with a different key) or deleted entries
	//note that the lock of this hash_entry_t must be initialized first
	pthread_mutex_lock(&entry->lock);
	int itr = 0;
	while((entry->id == (unsigned) -1 || (entry->key && 
			entry->key != op->key)) && itr < h->table_size){
		
		ctr = (ctr + 1) % h->table_size;
		pthread_mutex_unlock(&entry->lock);
		entry = h->table + ctr; 
		pthread_mutex_lock(&entry->lock);
		itr++;
	} 
	if(entry->key == op->key){
		op->datalen = entry->datalen;
		op->data = entry->data;
		//printf("%lu LOOKUP %lu SUCCESS\n", op->id, op->key);
		pthread_mutex_unlock(&entry->lock);
		return 0;
	}
	//printf("%lu LOOKUP %lu FAILED\n", op->id, op->key);
	pthread_mutex_unlock(&entry->lock);
  	return -1;    // Failed
}

int insert_update(hash_t *h, op_t *op) {
	//before updating the hash table entry, lock must be secured
	//after updating the hash tables entry, lock must be released
	unsigned ctr;
	unsigned hashval = hashfunc(op->key, h->table_size);
	hash_entry_t *entry = h->table + hashval;
	
	//assert(h && h->used < h->table_size);
	//assert(op && op->key);

	ctr = hashval;
		//note that two entreis in the hash table cannot have the same key
		//loops until finds an empty spot or an entry with the same key
	pthread_mutex_lock(&entry->lock);
	int itr = 0;
	while((entry->id == (unsigned) -1 || (entry->key && entry->key != op->key)) && itr < h->table_size){
		ctr = (ctr + 1) % h->table_size;
		pthread_mutex_unlock(&entry->lock);
		entry = h->table + ctr; 
		pthread_mutex_lock(&entry->lock);
		itr++;
	}

	if (itr == h->table_size) {
		//key was not found and no empty position remaining
		//insert at first deleted position
		itr = 0;
		while(entry->key && entry->key != op->key && itr < h->table_size){
         	ctr = (ctr + 1) % h->table_size;
			pthread_mutex_unlock(&entry->lock);
         	entry = h->table + ctr;
			pthread_mutex_lock(&entry->lock);
			itr++;
   		} 
	}

	//assert(ctr != hashval - 1);

	if(entry->key == op->key){  //It is an update
		entry->id = op->id;
		entry->datalen = op->datalen;
		entry->data = op->data;
		//printf("%lu UPDATE %lu SUCCESS\n", op->id, op->key);
		pthread_mutex_unlock(&entry->lock);
		return 0;
	}
	//assert(!entry->key);   // Fresh insertion
	
	entry->id = op->id;
	entry->datalen = op->datalen;
	entry->key = op->key;
	entry->data = op->data;
	//printf("%lu INSERT %lu SUCCESS\n", op->id, op->key);
	pthread_mutex_unlock(&entry->lock);

	atomic_add(&h->used, 1);
	return 0; 
  	//return -1;    // Failed
}

int purge_key(hash_t *h, op_t *op) {
	//before updating the hash table entry, lock must be secured
	//after updating the hash tables entry, lock must be released
	unsigned ctr;
	unsigned hashval = hashfunc(op->key, h->table_size);
	hash_entry_t *entry = h->table + hashval;
	
	ctr = hashval;
	//basically the same as search
	pthread_mutex_lock(&entry->lock);
	int itr = 0;
	while((entry->id == (unsigned) -1 || (entry->key && 
			entry->key != op->key)) && itr < h->table_size){

			ctr = (ctr + 1) % h->table_size;
			pthread_mutex_unlock(&entry->lock);
			entry = h->table + ctr;
			pthread_mutex_lock(&entry->lock);
			itr++;
	} 

	if(entry->key == op->key){  //Found. purge it
		entry->id = (unsigned) -1;  //Empty but deleted
		entry->key = 0;
		entry->datalen = 0;
		entry->data = NULL;
		//printf("%lu DELETE %lu FOUND\n", op->id, op->key);
		pthread_mutex_unlock(&entry->lock);
		atomic_add(&h->used, -1);
		return 0;
	}
	//printf("%lu DELETE %lu NOTFOUND\n", op->id, op->key);
	pthread_mutex_unlock(&entry->lock);
  	return -1;    // Bogus purge
}
