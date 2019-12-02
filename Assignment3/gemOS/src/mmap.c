/*
CS330 Assignment 3

Submitted By:
Name: Ayush Kumar
Roll No: 170195
File: mmap.c
*/

#include<types.h>
#include<mmap.h>
#include<page.h>
#include<entry.h>

/**
 * Function will invoked whenever there is page fault. (Lazy allocation)
 * 
 * For valid acess. Map the physical page 
 * Return 1
 * 
 * For invalid access,
 * Return -1. 
 */

u64 ceil(u64 a, u64 b) {
	if (a % b == 0) return a / b;
	else return a / b + 1;
}

void create_page_to_pfn_mapping(struct exec_context *current, u64 curr_addr, int length, int prot) {
	for(u64 page_addr = curr_addr; page_addr < curr_addr + length; page_addr += PAGE_SIZE) {
		map_physical_page(current->pgd << PAGE_SHIFT, page_addr, prot, 0);
	}
}

void delete_page_to_pfn_mapping(struct exec_context *current, u64 curr_addr, int length) {
	for(u64 page_addr = curr_addr; page_addr < curr_addr + length; page_addr += PAGE_SIZE) {
		u64* pte = get_user_pte(current, page_addr, 0);
		if (pte) {
			u64 pfn = (*pte & FLAG_MASK) >> PAGE_SHIFT;
			struct pfn_info* curr_pfn_info = get_pfn_info(pfn);
			if (get_pfn_info_refcount(curr_pfn_info) > 1) {
				decrement_pfn_info_refcount(curr_pfn_info);
			} else {
				os_pfn_free(USER_REG, (*pte >> PTE_SHIFT) & 0xFFFFFFFF);
			}
			*pte = 0;
			asm volatile("invlpg (%0);" :: "r"(page_addr) : "memory");
		}
	}
}

void modify_page_to_pfn_mapping(struct exec_context *current, u64 curr_addr, int length, int prot) {
	//only the last 3 bits are access flags
	//need to modify the last 3 bits of the page table entry
	for(u64 page_addr = curr_addr; page_addr < curr_addr + length; page_addr += PAGE_SIZE) {
		u64* pte = get_user_pte(current, page_addr, 0);
		struct pfn_info* curr_pfn_info = get_pfn_info((*pte & FLAG_MASK) >> PAGE_SHIFT);
		if (get_pfn_info_refcount(curr_pfn_info) > 1) {
			//this page is being shared my multiple processed
			//hence don't change its access flags
			continue;
		}
		if (prot & PROT_WRITE) {
			*pte |= (1u << 1);
		} else {
			*pte &= ~(1u << 1); 
		}
		asm volatile("invlpg (%0);" :: "r"(page_addr) : "memory");
	}
}

int vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
{
	addr = addr - addr % PAGE_SIZE;
	struct vm_area* curr_vm_area = current->vm_area;
	while (curr_vm_area) {
		if (curr_vm_area->vm_start <= addr && addr < curr_vm_area->vm_end) {
			break;
		}
		curr_vm_area = curr_vm_area->vm_next;
	}
	if (!curr_vm_area) {
		//page at address addr doesn't exist in the vm_area maps
		return -EINVAL;
	}
    if (error_code == 4) {
    	//read page fault
    	//means the page at addr doesn't have read permission means page doesn't even exist
    	map_physical_page(current->pgd << PAGE_SHIFT, addr, curr_vm_area->access_flags, 0);
    } else if (error_code == 6) {
    	//write page fault
    	//means the page at addr doesn't have write permission means page doesn't have write permission
    	if (!(curr_vm_area->access_flags & PROT_WRITE)) return -EINVAL;
    	map_physical_page(current->pgd << PAGE_SHIFT, addr, curr_vm_area->access_flags, 0);
    }
    return 1;
}

int vm_area_mprotect_dry_run(struct exec_context *current, u64 addr, int length, int prot) {
	//this function does a dry run on the vm_areas and computes
	//the number of vm_areas that would be created as a result of the mprotect call
	int num_new_vm_areas = 0;
	struct vm_area* curr_vm_area = current->vm_area;
	struct vm_area* prev_vm_area = NULL;
	int curr_prot = curr_vm_area->access_flags;
	int prev_prot = 0xFFFFFFFF;
	u64 curr_addr = MMAP_AREA_START;
	while (curr_addr <= addr + length) {
		if ((curr_addr <= addr && (!curr_vm_area || (curr_vm_area && addr < curr_vm_area->vm_start))) ||
		(addr <= curr_addr && curr_addr < addr + length && (!curr_vm_area || (curr_vm_area->vm_start - curr_addr > 0)))) {
			//the mprotect call will fail anyway
			return 0;
		} else if (addr <= curr_vm_area->vm_start && curr_vm_area->vm_end <= addr + length) {
			if (prot != curr_vm_area->access_flags) {
				//modify this whole vm_area
				curr_prot = prot;
			}
		} else if (addr <= curr_vm_area->vm_start && curr_vm_area->vm_start < addr + length) {
			if (prot != curr_vm_area->access_flags) {
				//modify the front part of this vm_area
				num_new_vm_areas++;
				curr_prot = prot;
			}
		} else if (curr_vm_area->vm_start < addr && addr + length < curr_vm_area->vm_end) {
			if (prot != curr_vm_area->access_flags) {
				//modify the middle part of this vm_area
				return 2;
			}
		} else if (curr_vm_area->vm_start < addr && addr < curr_vm_area->vm_end) {
			if (prot != curr_vm_area->access_flags) {
				//modify the end part of this vm_area
				num_new_vm_areas++;
				prev_prot = 0xFFFFFFFF;
				curr_prot = prot;
			}
		}
		//merge curr_vm_area with the prev_vm_area if required
		if (prev_vm_area && curr_vm_area && prev_vm_area->vm_end == curr_vm_area->vm_start &&
			prev_prot == curr_prot) {
			//merge current with previous
			num_new_vm_areas--;
		}
		curr_addr = curr_vm_area->vm_end;
		prev_vm_area = curr_vm_area;
		prev_prot = curr_prot;
		curr_vm_area = curr_vm_area->vm_next;
		curr_prot = curr_vm_area->access_flags;
	}
	return num_new_vm_areas;
}

/**
 * mprotect System call Implementation.
 */
int vm_area_mprotect(struct exec_context *current, u64 addr, int length, int prot)
{
	prot &= (PROT_READ | PROT_WRITE);
	if (length < 0) {
		return -EINVAL;
	}
	if (length == 0) {
		return 0;
	}
	//length needs to be a multiple of PAGE_SIZE
	length = ceil(length, PAGE_SIZE) * PAGE_SIZE;
	//address needs to be a multiple of PAGE_SIZE
	if (addr < MMAP_AREA_START || (addr - MMAP_AREA_START) % PAGE_SIZE != 0 || MMAP_AREA_END <= addr) {
		return -EINVAL;
	}
	//int num_new_vm_areas = vm_area_mprotect_dry_run(current, addr, length, prot);
	//if (stats->num_vm_area + num_new_vm_areas > 128) return -EINVAL;
	struct vm_area* curr_vm_area = current->vm_area;
	struct vm_area* prev_vm_area = NULL;
	u64 curr_addr = MMAP_AREA_START;
	while(curr_addr <= addr + length) {
		//printk("curr_addr = %x, addr = %x, addr+length = %x\n", curr_addr, addr, addr+length);
		//check for any overlap with the curr_vm_area
		if ((curr_addr <= addr && (!curr_vm_area || (curr_vm_area && addr < curr_vm_area->vm_start))) ||
			(addr <= curr_addr && (curr_addr < addr + length) && (!curr_vm_area || (curr_vm_area->vm_start - curr_addr > 0)))) {
			return -EINVAL;
		} else if (addr <= curr_vm_area->vm_start && curr_vm_area->vm_end <= addr + length) {
			if (prot != curr_vm_area->access_flags) {
				//modify this whole vm_area
				curr_vm_area->access_flags = prot;
				modify_page_to_pfn_mapping(current, curr_vm_area->vm_start, curr_vm_area->vm_end - curr_vm_area->vm_start, prot);
			}
			//this may be needed to be merged with the previous one or the next one or both
		} else if (addr <= curr_vm_area->vm_start && curr_vm_area->vm_start < addr + length) {
			if (prot != curr_vm_area->access_flags) {
				//modify the front part of this vm_area
				struct vm_area* new_vm_area = alloc_vm_area();
				new_vm_area->vm_start = curr_vm_area->vm_start;
				new_vm_area->vm_end = addr + length;
				new_vm_area->access_flags = prot;
				new_vm_area->vm_next = curr_vm_area;

				curr_vm_area->vm_start = addr + length;
				if (prev_vm_area) prev_vm_area->vm_next = new_vm_area;
				else current->vm_area = new_vm_area;
				curr_vm_area = new_vm_area;
				modify_page_to_pfn_mapping(current, new_vm_area->vm_start, new_vm_area->vm_end - new_vm_area->vm_start, prot);
			}
			//this may be needed to be merged with the previous one
		} else if (curr_vm_area->vm_start < addr && addr + length < curr_vm_area->vm_end) {
			if (prot != curr_vm_area->access_flags) {
				//modify the middle part of this vm_area i.e. add two more vm_areas
				struct vm_area* new_mid_vm_area = alloc_vm_area();
				struct vm_area* new_end_vm_area = alloc_vm_area();
				new_mid_vm_area->vm_start = addr;
				new_mid_vm_area->vm_end = addr + length;
				new_mid_vm_area->access_flags = prot;
				new_mid_vm_area->vm_next = new_end_vm_area;

				new_end_vm_area->vm_start = addr + length;
				new_end_vm_area->vm_end = curr_vm_area->vm_end;
				new_end_vm_area->access_flags = curr_vm_area->access_flags;
				new_end_vm_area->vm_next = curr_vm_area->vm_next;

				curr_vm_area->vm_next = new_mid_vm_area;
				curr_vm_area->vm_end = addr;
				curr_vm_area = new_end_vm_area;
				modify_page_to_pfn_mapping(current, new_mid_vm_area->vm_start, new_mid_vm_area->vm_end - new_mid_vm_area->vm_start, prot);
			}
		} else if (curr_vm_area->vm_start < addr && addr < curr_vm_area->vm_end) {
			if (prot != curr_vm_area->access_flags) {
				//modify the end part of this vm_area
				struct vm_area* new_vm_area = alloc_vm_area();
				new_vm_area->vm_start = addr;
				new_vm_area->vm_end = curr_vm_area->vm_end;
				new_vm_area->access_flags = prot;
				new_vm_area->vm_next = curr_vm_area->vm_next;

				curr_vm_area->vm_next = new_vm_area;
				curr_vm_area->vm_end = addr;
				curr_vm_area = new_vm_area;
				modify_page_to_pfn_mapping(current, new_vm_area->vm_start, new_vm_area->vm_end - new_vm_area->vm_start, prot);
			}
			//this may be needed to be merged with the next one 
		}
		//currently, curr_vm_area points to the vm_area whose prot was changed
		//merge curr_vm_area with the prev_vm_area if required
		if (prev_vm_area && curr_vm_area && prev_vm_area->vm_end == curr_vm_area->vm_start &&
			prev_vm_area->access_flags == curr_vm_area->access_flags) {
			//merge current with previous
			prev_vm_area->vm_end = curr_vm_area->vm_end;
			prev_vm_area->vm_next = curr_vm_area->vm_next;
			dealloc_vm_area(curr_vm_area);
			curr_vm_area = prev_vm_area;
		}
		if (curr_addr == addr + length) {
			break;
		}
		curr_addr = curr_vm_area->vm_end;
		prev_vm_area = curr_vm_area;
		curr_vm_area = curr_vm_area->vm_next;
	}
    //successful
    return 0;
}
/**
 * mmap system call implementation.
 */
//note that mapping has to be done in the region [MMAP_AREA_START, MMAP_AREA_END)
int create_mapping(struct exec_context *current, 
	struct vm_area* prev_vm_area, u64 curr_addr, struct vm_area* curr_vm_area, int length, int prot) {
	if (prev_vm_area && prev_vm_area->access_flags == prot && prev_vm_area->vm_end == curr_addr) {
		//the previous element exists
		//merge prev with the new one
		if (curr_vm_area && curr_vm_area->access_flags == prot && 
			curr_addr + length == curr_vm_area->vm_start) {
			//merge prev, curr, and the new into one vm_area
			prev_vm_area->vm_end = curr_vm_area->vm_end;
			prev_vm_area->vm_next = curr_vm_area->vm_next;
			dealloc_vm_area(curr_vm_area);
		} else {
			prev_vm_area->vm_end += length;
		}
	} else if (curr_vm_area && curr_vm_area->access_flags == prot &&
		curr_addr + length == curr_vm_area->vm_start) {
		//merge the new one with curr
		curr_vm_area->vm_start = curr_addr;
	} else {
		if (stats->num_vm_area >= 128) return 0;
		struct vm_area* new_vm_area = alloc_vm_area();
		new_vm_area->vm_start = curr_addr;
		new_vm_area->vm_end = curr_addr + length;
		new_vm_area->access_flags = prot;
		new_vm_area->vm_next = curr_vm_area;
		if (prev_vm_area) prev_vm_area->vm_next = new_vm_area;
		else current->vm_area = new_vm_area;
	}
	return 1;
	//return curr_addr;
}

long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags)
{
	prot &= (PROT_READ | PROT_WRITE);
	flags &= (MAP_FIXED | MAP_POPULATE);
	if (length < 0) {
		return -EINVAL;
	}
	if (length == 0) {
		return addr;
	}
	length = ceil(length, PAGE_SIZE) * PAGE_SIZE;
	if (!addr) {
		//no address hint supplied
		//insert the vm_area at the first unoccupied location
		struct vm_area* curr_vm_area = current->vm_area;
		struct vm_area* prev_vm_area = NULL;
		u64 curr_addr = MMAP_AREA_START;
		while(1) {
			if (!curr_vm_area) {
				//this is at the end of the list
				if (curr_addr + length <= MMAP_AREA_END) {
					//alloc here, there is enough space to alloc
					if (!create_mapping(current, prev_vm_area, curr_addr, curr_vm_area, length, prot)) return -EINVAL;
					if (flags & MAP_POPULATE) {
						//allocate physical page for each of the pages in this vm_area
						create_page_to_pfn_mapping(current, curr_addr, length, prot);
					}
					return curr_addr;
				} else {
					//not enough space to alloc even at the end of the list
					return -EINVAL;
				}
			} else if (curr_addr + length <= curr_vm_area->vm_start) {
				//enough space available to create the new mapping
				//insert here or extend the previous one or pre-extend the current one
				if (!create_mapping(current, prev_vm_area, curr_addr, curr_vm_area, length, prot)) return -EINVAL;
				if (flags & MAP_POPULATE) {
					//allocate physical page for each of the pages in this vm_area
					create_page_to_pfn_mapping(current, curr_addr, length, prot);
				}
				return curr_addr;
			} else {
				curr_addr = curr_vm_area->vm_end;
				prev_vm_area = curr_vm_area;
				curr_vm_area = curr_vm_area->vm_next;
			}
		}
	} else {
		//address hint is supplied
		//address needs to be a multiple of PAGE_SIZE
		if (addr < MMAP_AREA_START || (addr - MMAP_AREA_START) % PAGE_SIZE != 0 || MMAP_AREA_END <= addr) {
			return -EINVAL;
		}
		//first need to check if this address is available
		//if available insert/merge there else insert without address hint
		struct vm_area* curr_vm_area = current->vm_area;
		struct vm_area* prev_vm_area = NULL;
		u64 curr_addr = MMAP_AREA_START;
		while(1) {
			if (curr_addr <= addr && ((!curr_vm_area && addr + length <= MMAP_AREA_END) ||
				(curr_vm_area && addr + length <= curr_vm_area->vm_start))) {
				//allocate here
				if (!create_mapping(current, prev_vm_area, addr, curr_vm_area, length, prot)) return -EINVAL;
				if (flags & MAP_POPULATE) {
					//allocate physical page for each of the pages in this vm_area
					create_page_to_pfn_mapping(current, addr, length, prot);
				}
				return addr;
			} else if (curr_vm_area && curr_vm_area->vm_end <= addr) {
				curr_addr = curr_vm_area->vm_end;
				prev_vm_area = curr_vm_area;
				curr_vm_area = curr_vm_area->vm_next;
			} else {
				//the given mapping cannot be allocated at addr
				//try allocating at some other address
				if (flags & MAP_FIXED) return -EINVAL;
				else return vm_area_map(current, NULL, length, prot, flags);
			}
		}
	}
}
/**
 * munmap system call implementations
 */

int vm_area_unmap(struct exec_context *current, u64 addr, int length)
{
	if (length < 0) {
		return -EINVAL;
	}
	if (length == 0) {
		return 0;
	}
	//length needs to be a multiple of PAGE_SIZE
	length = ceil(length, PAGE_SIZE) * PAGE_SIZE;
	//address needs to be a multiple of PAGE_SIZE
	if (addr < MMAP_AREA_START || (addr - MMAP_AREA_START) % PAGE_SIZE != 0 || MMAP_AREA_END <= addr) {
		return -EINVAL;
	}
	struct vm_area* curr_vm_area = current->vm_area;
	struct vm_area* prev_vm_area = NULL;
	while(curr_vm_area && curr_vm_area->vm_start < addr + length) {
		//check for any overlap with the curr_vm_area
		if (addr <= curr_vm_area->vm_start && curr_vm_area->vm_end <= addr + length) {
			//remove this whole vm_area
			if (prev_vm_area) prev_vm_area->vm_next = curr_vm_area->vm_next;
			else current->vm_area = curr_vm_area->vm_next;
			delete_page_to_pfn_mapping(current, curr_vm_area->vm_start, curr_vm_area->vm_end - curr_vm_area->vm_start);
			dealloc_vm_area(curr_vm_area);
			if (prev_vm_area) curr_vm_area = prev_vm_area;
			else {
				curr_vm_area = current->vm_area;
				continue;
			}
		} else if (addr <= curr_vm_area->vm_start) {
			//remove the front part of this vm_area
			delete_page_to_pfn_mapping(current, curr_vm_area->vm_start, addr + length - curr_vm_area->vm_start);
			curr_vm_area->vm_start = addr + length;
		} else if (curr_vm_area->vm_start < addr && addr + length < curr_vm_area->vm_end) {
			//remove the middle part of this vm_area
			if (stats->num_vm_area >= 128) return -EINVAL;
			struct vm_area* new_vm_area = alloc_vm_area();
			new_vm_area->vm_start = addr + length;
			new_vm_area->vm_end = curr_vm_area->vm_end;
			new_vm_area->access_flags = curr_vm_area->access_flags;
			new_vm_area->vm_next = curr_vm_area->vm_next; 

			curr_vm_area->vm_next = new_vm_area;
			curr_vm_area->vm_end = addr;
			curr_vm_area = new_vm_area;
			delete_page_to_pfn_mapping(current, addr, length);
		} else if (curr_vm_area->vm_start < addr && addr < curr_vm_area->vm_end) {
			//remove the end part of this vm_area
			delete_page_to_pfn_mapping(current, addr, curr_vm_area->vm_end - addr);
			curr_vm_area->vm_end = addr;
		}
		prev_vm_area = curr_vm_area;
		curr_vm_area = curr_vm_area->vm_next;
	}
	//successfully unmapped areas
    return 0;
}