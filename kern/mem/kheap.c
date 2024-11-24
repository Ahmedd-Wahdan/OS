#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
#define MAX_NUM_OF_PAGES (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE


//bool located[MAX_NUM_OF_PAGES] = {0};
//bool end[MAX_NUM_OF_PAGES] = {0};
//struct free free_pages_arr[MAX_NUM_OF_PAGES] = {0};


struct pages {
    bool allocated;
    uint32 va;
    int psize;

};
struct pages Pages_arr[MAX_NUM_OF_PAGES];



//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	 //TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
    // Write your code here, remove the panic and write your code
   //panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");

	initSizeToAllocate = ROUNDUP(initSizeToAllocate, PAGE_SIZE);
	if(initSizeToAllocate + daStart > daLimit)
		panic(" *Error While Initializing kheap Dynamic Allocator* ");

	setStartLimit(daStart);
	setSegementBrk(initSizeToAllocate + daStart);
	setHardLimit(ROUNDDOWN(daLimit,PAGE_SIZE));


	for (uint32 virtual_address = daStart; virtual_address < getSBrk(); virtual_address += PAGE_SIZE){
		struct FrameInfo *frame_info;
		 if(allocate_frame(&frame_info) == 0) {
		        map_frame(ptr_page_directory, frame_info, virtual_address, PERM_PRESENT  | PERM_WRITEABLE);
		        frame_info->VA=virtual_address;
		 }
		 else{
			 for (uint32 allocated_va = daStart; allocated_va < virtual_address; allocated_va += PAGE_SIZE){
				 unmap_frame(ptr_page_directory,allocated_va);

			 }
			 panic(" **Error While Initializing kheap Dynamic Allocator** ");
		 }
	}

	initialize_dynamic_allocator(daStart, initSizeToAllocate);
	return 0 ;
}

void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	// TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk

	uint32 size = numOfPages * PAGE_SIZE;
	uint32 hard_limit = getHardLimit();
	uint32 old_s_brk = getSBrk();
	uint32 new_s_brk = old_s_brk + size;
	if(size == 0)
		return (void*)old_s_brk;
	if(new_s_brk > hard_limit)
		return(void*) -1;
	else{
		for (uint32 va = old_s_brk; va < new_s_brk; va += PAGE_SIZE){
				struct FrameInfo *frame_info;
				 if(allocate_frame(&frame_info) == 0) {
				        map_frame(ptr_page_directory, frame_info, va, PERM_PRESENT  | PERM_WRITEABLE);
				        frame_info->VA=va;
				    }
				 else{
					 for (uint32 allocated_va = old_s_brk; allocated_va < va; allocated_va += PAGE_SIZE){
						 unmap_frame(ptr_page_directory,allocated_va);
					 }
					 return(void*) -1;
				 }
			}
	}

	setSegementBrk(new_s_brk);
	return (void*) old_s_brk;

}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator
void* kmalloc(unsigned int size)
{
    //[PROJECT'24.MS2] Implement this function
    // Write your code here, remove the panic and write your code
    // kpanic_into_prompt("kmalloc() is not implemented yet...!!");

    // Use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
    if (isKHeapPlacementStrategyFIRSTFIT()) {

        int max_size = KERNEL_HEAP_MAX - (HARD_LIMIT + PAGE_SIZE);

        if (size == 0||size>max_size) {
            return NULL;
        }

        if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {

            return alloc_block_FF((uint32) size);

        }

        int pgs_needed = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

        int count_wrabaad = 0;
        int firstpg = -1;
        uint32 MAX_PAGES = (KERNEL_HEAP_MAX - (getHardLimit() + PAGE_SIZE)) / PAGE_SIZE;
        for(uint32 i = 0; i < MAX_PAGES; i++){
                	if(Pages_arr[i].allocated == 0)
                	{
                		if(count_wrabaad == 0)
                		{
                			firstpg = i;
                		}

                		count_wrabaad++;

                		if(count_wrabaad == pgs_needed)
                		{
                			break;
                		}
                	}
                	else
                	{
                		count_wrabaad = 0;
                	}
                }

        if (count_wrabaad < pgs_needed) {
            return NULL;
        }

        uint32 va_start_address = HARD_LIMIT + PAGE_SIZE + (firstpg * PAGE_SIZE);
        uint32 va = va_start_address;

        int allct_res;
        int map_res;
        for (int i = firstpg; i < firstpg + pgs_needed; i++) {
            struct FrameInfo *frame;

           allct_res=allocate_frame(&frame);
           map_res=map_frame(ptr_page_directory, frame, va, PERM_PRESENT | PERM_WRITEABLE);

                Pages_arr[i].allocated = 1;
                frame->VA=va;
                va += PAGE_SIZE;
            if(map_res!=0||allct_res!=0) {
            	uint32 address = va_start_address;
            	for (int R = firstpg; R < i; R++)
            	{ frame->VA=0;
            	  unmap_frame(ptr_page_directory, address);
            	  address += PAGE_SIZE;
            	  Pages_arr[R].allocated = 0;
            	  Pages_arr[R].psize = 0;
            	 }
                return NULL;
            }
            Pages_arr[i].va = va_start_address;
            Pages_arr[i].psize = size;
        }


        return (void*) va_start_address;

    }
    else {
        return NULL;
    }
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

	if((uint32)virtual_address < getStartLimit() || (uint32)virtual_address >= KERNEL_HEAP_MAX)
	{
		panic("WHY?");
		return;
	}

	if((uint32)virtual_address >= getSBrk() && (uint32)virtual_address < getHardLimit() + PAGE_SIZE)
	{
		return;
	}

	if((uint32)virtual_address >= getStartLimit() && (uint32)virtual_address < getSBrk())
	{
		free_block(virtual_address);
		return;
	}
	else
	{

		uint32 va = (uint32)virtual_address;
		va = ROUNDDOWN(va, PAGE_SIZE);

		int page = (va - (getHardLimit() + PAGE_SIZE)) / PAGE_SIZE;
		if(Pages_arr[page].allocated == 0)
		{
			return;
		}

		va = Pages_arr[page].va;

		int allocated_pages = ROUNDUP(Pages_arr[page].psize, PAGE_SIZE) / PAGE_SIZE;

		for(int i = page; i < page + allocated_pages; i++)
		{
			unmap_frame(ptr_page_directory, va);
			va += PAGE_SIZE;
			Pages_arr[i].allocated = 0;
			Pages_arr[i].psize = 0;
		}

		return;
	}

	return;
}






unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
	uint32 *ptr_page_table;
		get_page_table(ptr_page_directory,virtual_address,&ptr_page_table);

		uint32 pagetblindex=PTX(virtual_address);

	if(ptr_page_table!=0&&(ptr_page_table[pagetblindex] & PERM_PRESENT)){
		uint32 frameadd=ptr_page_table[pagetblindex]&0xFFFFF000;
				uint32 offset= virtual_address& 0x00000FFF;
				unsigned int physicaladd=frameadd+offset;
				return physicaladd;
	}
	else{
		return 0;
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================


	struct FrameInfo * frame=NULL;
	uint32 offset= physical_address& 0xFFF;
		frame=to_frame_info(physical_address);
	//	if(frame!=NULL&&frame->references!=0&&frame->VA!=0){

	        if (frame->references==0||frame==NULL){
	        	return 0;}
	        else{
	        	unsigned int va =frame->VA;
		    return (va|offset);}

	        return 0;


}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
