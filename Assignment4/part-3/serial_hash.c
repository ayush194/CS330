#include "common.h"

int read_op(struct input_manager *in, op_t *op, int tnum)
{
   unsigned size = sizeof(op_t);
   memcpy(op, in->curr, size - sizeof(unsigned long));  //Copy till data ptr     
   if(op->op_type == GET || op->op_type == DEL){
       in->curr += size - sizeof(op->datalen) - sizeof(op->data);
   }else if(op->op_type == PUT){
       in->curr += size - sizeof(op->data);
       op->data = in->curr;
       in->curr += op->datalen;
   }else{
       assert(0);
   }
   if(in->curr > in->data + in->size)
        return -1;
    
   in->being_processed[tnum] = op;
   return 0; 
}

void done_one(struct input_manager *in, int tnum)
{
   in->being_processed[tnum] = NULL; 
}

int lookup(hash_t *h, op_t *op)
{
  unsigned ctr;
  unsigned hashval = hashfunc(op->key, h->table_size);
  hash_entry_t *entry = h->table + hashval;
  ctr = hashval;
  //searches for the first entry which is either empty or contains the same key
  int itr = 0;
  while((entry->id == (unsigned) -1 || (entry->key && 
			entry->key != op->key)) && itr < h->table_size){
      
      ctr = (ctr + 1) % h->table_size;
      entry = h->table + ctr; 
      itr++;
  } 
 if(entry->key == op->key){
      op->datalen = entry->datalen;
      op->data = entry->data;
      printf("%lu LOOKUP %lu SUCCESS\n", op->id, op->key);
      return 0;
 }
 printf("%lu LOOKUP %lu FAILED\n", op->id, op->key);
 return -1;
}

int insert_update(hash_t *h, op_t *op)
{
   unsigned ctr;
   unsigned hashval = hashfunc(op->key, h->table_size);
   hash_entry_t *entry = h->table + hashval;
   
   assert(h && h->used < h->table_size);
   assert(op && op->key);

   ctr = hashval;
    //note that two entreis in the hash table cannot have the same key
    //loops until finds an empty spot or an entry with the same key
    int itr = 0;
   while((entry->id == (unsigned) -1 || (entry->key && entry->key != op->key)) && itr < h->table_size){
         ctr = (ctr + 1) % h->table_size;
         entry = h->table + ctr; 
         itr++;
   } 

   if (itr == h->table_size) {
		//key was not found and no empty position remaining
		//insert at first deleted position
		itr = 0;
		while(entry->key && entry->key != op->key && itr < h->table_size){
         	ctr = (ctr + 1) % h->table_size;
         	entry = h->table + ctr;
			itr++;
   		} 
	}

  assert(ctr != hashval - 1);

  if(entry->key == op->key){  //It is an update
      entry->id = op->id;
      entry->datalen = op->datalen;
      entry->data = op->data;
      printf("%lu UPDATE %lu SUCCESS\n", op->id, op->key);
      return 0;
  }
  assert(!entry->key);   // Fresh insertion
 
  entry->id = op->id;
  entry->datalen = op->datalen;
  entry->key = op->key;
  entry->data = op->data;
  printf("%lu INSERT %lu SUCCESS\n", op->id, op->key);
  
  h->used++;
  return 0; 
}

int purge_key(hash_t *h, op_t *op)
{
   unsigned ctr;
   unsigned hashval = hashfunc(op->key, h->table_size);
   hash_entry_t *entry = h->table + hashval;
   
   ctr = hashval;
   //basically the same as search
   int itr = 0;
   while((entry->id == (unsigned) -1 || (entry->key && 
			entry->key != op->key)) && itr < h->table_size){

         ctr = (ctr + 1) % h->table_size;
         entry = h->table + ctr; 
         itr++;
   } 

   if(entry->key == op->key){  //Found. purge it
      entry->id = (unsigned) -1;  //Empty but deleted
      entry->key = 0;
      entry->datalen = 0;
      entry->data = NULL;
      h->used--;
      printf("%lu DELETE %lu FOUND\n", op->id, op->key);
      return 0;
   }
   printf("%lu DELETE %lu NOTFOUND\n", op->id, op->key);
  return -1;    // Bogus purge
}
