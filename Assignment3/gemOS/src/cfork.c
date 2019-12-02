/*
CS330 Assignment 3

Submitted By:
Name: Ayush Kumar
Roll No: 170195
File: cfork.c
*/

#include <cfork.h>
#include <page.h>
#include <mmap.h>



/* You need to implement cfork_copy_mm which will be called from do_cfork in entry.c. Don't remove copy_os_pts()*/

void cfork_copy_mm(struct exec_context *child, struct exec_context *parent)
{
	void *os_addr;
	u64 vaddr; 
	struct mm_segment *seg;

	child->pgd = os_pfn_alloc(OS_PT_REG);

	os_addr = osmap(child->pgd);
	bzero((char *)os_addr, PAGE_SIZE);

	//CODE segment
	seg = &parent->mms[MM_SEG_CODE];
	for(vaddr = seg->start; vaddr < seg->next_free; vaddr += PAGE_SIZE) {
		u64 *parent_pte = get_user_pte(parent, vaddr, 0);
		if(parent_pte) {
			install_ptable((u64)os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
			*parent_pte &= ~(1u << 1);
			asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
			u64* child_pte = get_user_pte(child, vaddr, 0);
			*child_pte &= ~(1u << 1);
			asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
			struct pfn_info* curr_pfn_info= get_pfn_info((*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
			increment_pfn_info_refcount(curr_pfn_info);
		}
	}
	//RODATA segment
	seg = &parent->mms[MM_SEG_RODATA];
	for(vaddr = seg->start; vaddr < seg->next_free; vaddr += PAGE_SIZE) {
		u64 *parent_pte = get_user_pte(parent, vaddr, 0);
		if(parent_pte) {
			install_ptable((u64)os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
			*parent_pte &= ~(1u << 1);
			asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
			u64* child_pte = get_user_pte(child, vaddr, 0);
			*child_pte &= ~(1u << 1);
			asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
			struct pfn_info* curr_pfn_info = get_pfn_info((*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
			increment_pfn_info_refcount(curr_pfn_info);
		}
	}

	//DATA segment
	seg = &parent->mms[MM_SEG_DATA];
	for(vaddr = seg->start; vaddr < seg->next_free; vaddr += PAGE_SIZE) {
		u64 *parent_pte = get_user_pte(parent, vaddr, 0);
		if(parent_pte) {
			install_ptable((u64)os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
			*parent_pte &= ~(1u << 1);
			asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
			u64* child_pte = get_user_pte(child, vaddr, 0);
			*child_pte &= ~(1u << 1);
			asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
			struct pfn_info* curr_pfn_info = get_pfn_info((*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
			increment_pfn_info_refcount(curr_pfn_info); 
		}
	}

	//STACK segment
	seg = &parent->mms[MM_SEG_STACK];
	for(vaddr = seg->end - PAGE_SIZE; vaddr >= seg->next_free; vaddr -= PAGE_SIZE) {
		u64 *parent_pte = get_user_pte(parent, vaddr, 0); 
		if(parent_pte){
			u64 pfn = install_ptable((u64)os_addr, seg, vaddr, 0);  //Returns the blank page  
			pfn = (u64)osmap(pfn);
			memcpy((char *)pfn, (char *)(*parent_pte & FLAG_MASK), PAGE_SIZE); 
		}
	}

	//copy vm_area from parent's context
	child->vm_area = NULL;
	struct vm_area* child_prev_vm_area = NULL;
	struct vm_area* parent_curr_vm_area = parent->vm_area;
	while(parent_curr_vm_area) {
		for(vaddr = parent_curr_vm_area->vm_start; vaddr < parent_curr_vm_area->vm_end; vaddr += PAGE_SIZE) {
			u64 *parent_pte = get_user_pte(parent, vaddr, 0);
			if(parent_pte) {
				install_ptable((u64)os_addr, seg, vaddr, (*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
				*parent_pte &= ~(1u << 1);
				asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
				u64* child_pte = get_user_pte(child, vaddr, 0);
				*child_pte &= ~(1u << 1);
				asm volatile("invlpg (%0);" :: "r"(vaddr) : "memory");
				struct pfn_info* curr_pfn_info = get_pfn_info((*parent_pte & FLAG_MASK) >> PAGE_SHIFT);
				increment_pfn_info_refcount(curr_pfn_info); 
			}
		}

		struct vm_area* new_vm_area = alloc_vm_area();
		new_vm_area->vm_start = parent_curr_vm_area->vm_start;
		new_vm_area->vm_end = parent_curr_vm_area->vm_end;
		new_vm_area->access_flags = parent_curr_vm_area->access_flags;
		new_vm_area->vm_next = NULL;
		if (!child_prev_vm_area) child->vm_area = new_vm_area;
		else child_prev_vm_area->vm_next = new_vm_area;
		child_prev_vm_area = new_vm_area;
		parent_curr_vm_area = parent_curr_vm_area->vm_next;
	}
	//the mm_seg from parent's context has already been copied to the child context 

	copy_os_pts(parent->pgd, child->pgd);
	return; 
}

/* You need to implement cfork_copy_mm which will be called from do_vfork in entry.c.*/
void vfork_copy_mm(struct exec_context *child, struct exec_context *parent)
{
	parent->state = WAITING;
	//child->pgd already equals parent->pgd since its context was copid from parent
	//hence they already share the full page_table
	//now need to ensure that the child doesn't change the stack
	struct mm_segment* parent_stack_seg = &parent->mms[MM_SEG_STACK];
	struct mm_segment* child_stack_seg = &child->mms[MM_SEG_STACK];
	u64 stack_size = parent_stack_seg->end - parent_stack_seg->next_free;
	child_stack_seg->end -= stack_size;
	child_stack_seg->next_free -= stack_size;
	for(u64 page_addr = child_stack_seg->next_free; page_addr < child_stack_seg->end; page_addr += PAGE_SIZE) {
		u64* pte = get_user_pte(parent, page_addr + stack_size, 0);
		u64 parent_pfn_addr = (*pte & FLAG_MASK);
		u64 child_pfn = map_physical_page(child->pgd << PAGE_SHIFT, page_addr, child_stack_seg->access_flags, 0);
		memcpy((char*)(child_pfn << PAGE_SHIFT), (char*)parent_pfn_addr, PAGE_SIZE);
	}
	child->regs.entry_rsp -= stack_size;
	child->regs.rbp -= stack_size;
    return;
}

/*You need to implement handle_cow_fault which will be called from do_page_fault 
incase of a copy-on-write fault

* For valid acess. Map the physical page 
 * Return 1
 * 
 * For invalid access,
 * Return -1. 
*/

int handle_cow_fault(struct exec_context *current, u64 cr2)
{
	//page align cr2
	cr2 = cr2 - cr2 % PAGE_SIZE;
	//error code 7
	struct vm_area* curr_vm_area = current->vm_area;
	struct mm_segment* curr_mm_seg = NULL;
	while (curr_vm_area) {
		if (curr_vm_area->vm_start <= cr2 && cr2 < curr_vm_area->vm_end) {
			break;
		}
		curr_vm_area = curr_vm_area->vm_next;
	}
	for(int i = 0; i < MAX_MM_SEGS - 1; i++) {
		if (current->mms[i].start <= cr2 && cr2 < current->mms[i].end) {
			curr_mm_seg = &(current->mms[i]);
			break;
		}
	}
	u32 access_flags = 0;
	if (curr_vm_area) access_flags = curr_vm_area->access_flags;
	else if (curr_mm_seg) access_flags = curr_mm_seg->access_flags;
	else {
		//page at address addr doesn't exist in the vm_area maps or mm_seg maps
		return -EINVAL;
	}
	if (!(access_flags & PROT_WRITE)) return -EINVAL;
	u64* curr_pte = get_user_pte(current, cr2, 0);
	struct pfn_info* curr_pfn = get_pfn_info((*curr_pte & FLAG_MASK) >> PAGE_SHIFT);
	if (get_pfn_info_refcount(curr_pfn) == 2) {
		decrement_pfn_info_refcount(curr_pfn);
		u64 new_pfn = map_physical_page(current->pgd << PAGE_SHIFT, cr2, access_flags, 0);
		new_pfn = new_pfn << PAGE_SHIFT;
		memcpy((char *)new_pfn, (char *)(*curr_pte & FLAG_MASK), PAGE_SIZE);
	} else {
		*curr_pte |= (1u << 1);
		asm volatile("invlpg (%0);" :: "r"(cr2) : "memory");
	}
    return 1;
}

/* You need to handle any specific exit case for vfork here, called from do_exit*/
void vfork_exit_handle(struct exec_context *ctx)
{
	struct exec_context* parent = get_ctx_by_pid(ctx->ppid);
	if (parent->pgd == ctx->pgd) {
		//ctx is parent's child
		struct mm_segment* child_stack_seg = &ctx->mms[MM_SEG_STACK];
		//deallocate the extra stack that was created
		for(u64 page_addr = child_stack_seg->next_free; page_addr < child_stack_seg->end; page_addr += PAGE_SIZE) {
			u64* pte = get_user_pte(ctx, page_addr, 0);
			u64 pfn = (*pte & FLAG_MASK) >> PAGE_SHIFT;
			//os_pfn_free(USER_REG, (*pte >> PTE_SHIFT) & 0xFFFFFFFF);
			os_pfn_free(USER_REG, pfn);
			*pte = 0;
			asm volatile("invlpg (%0);" :: "r"(page_addr) : "memory");
		}
		parent->vm_area = ctx->vm_area;
		//parent needs to be in READY state now
		parent->state = READY;
	}
	return;
}