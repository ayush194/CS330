#include<page.h>
#include<lib.h>

//Returns the pfn_info corresponding to the PFN passed as index argument.
struct pfn_info * get_pfn_info(u32 index){
    struct pfn_info * p = ((struct pfn_info *)list_pfn_info.start)+index;
    return p;
}

//Sets refcount of pfn_info corresponding to the PFN passed as index argument.
void set_pfn_info(u32 index){
    struct pfn_info * p = ((struct pfn_info *)list_pfn_info.start)+index;
    p->refcount = 1;
}
void reset_pfn_info(u32 index){
    struct pfn_info * p = ((struct pfn_info *)list_pfn_info.start)+index;
    p->refcount = 0;
}

//Increments refcount of pfn_info object passed as argument.
void increment_pfn_info_refcount(struct pfn_info * p){
    p->refcount+=1;
    return;
}

//Decrements refcount of pfn_info object passed as argument.
void decrement_pfn_info_refcount(struct pfn_info * p){
    p->refcount-=1;
    return;
}

//Get refcount of pfn_info object passed as argument.
u8 get_pfn_info_refcount(struct pfn_info *p){
    return p->refcount;
}
