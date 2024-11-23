#include <inc/lib.h>

#define MAX_NUM_OF_UPAGES (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE

struct pages_u {
    bool allocated;
    uint32 va;
    int psize;

};
struct pages_u Pages_arr_user_heap[MAX_NUM_OF_UPAGES];

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	//return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy
	if(sys_isUHeapPlacementStrategyFIRSTFIT()){
		int max_size = USER_HEAP_MAX - (myEnv->hard_limit + PAGE_SIZE);
		if (size > max_size) {
			return NULL;
		}

		if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
			return alloc_block_FF((uint32) size);
		}

	     int pages_needed = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	     int counter= 0;
	     int first_page = -1;
	     uint32 MAX_PAGES =  max_size / PAGE_SIZE;

	     for(uint32 i = 0; i < MAX_PAGES; i++){
	    	 if(Pages_arr_user_heap[i].allocated == 0){
	    		 if(counter == 0){
	    			 first_page = i;
	    		 }
	    		 counter++;
	    		 if(counter== pages_needed){
	    			 break;
	    		 }
	    	 }
	         else{
	        	 counter= 0;
	         }

	     }
	     if (counter < pages_needed) {
	    	 return NULL;
	     }

	    for (int i = first_page; i < first_page + pages_needed; i++) {
	    	Pages_arr_user_heap[i].allocated = 1;
	    }

	    uint32 va_start_address = myEnv->hard_limit + PAGE_SIZE + (first_page * PAGE_SIZE);
	    sys_allocate_user_mem(va_start_address,ROUNDUP(size, PAGE_SIZE));
	    Pages_arr_user_heap[first_page].va = va_start_address;
	    Pages_arr_user_heap[first_page].psize = size;

	    return (void*) va_start_address;
	}
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	if (virtual_address == NULL) {
        return;
	}

	uint32 address = (uint32)virtual_address;

	if (address >= USER_HEAP_MAX || address < USER_HEAP_START) {
		panic("HEYYYY YOUUUU!!!!");
		return;
	}

	if (address >= myEnv->segment_break && address < myEnv->hard_limit + PAGE_SIZE) {
		return;
	}

	if (address >= myEnv->start && address < myEnv->segment_break) {
		free_block((void*)address);
		return;
	}

	int first_page = (address - (myEnv->hard_limit + PAGE_SIZE)) / PAGE_SIZE;

	if (!Pages_arr_user_heap[first_page].allocated) {
		return;
	}

	uint32 size = Pages_arr_user_heap[first_page].psize;

	//ROUNDUP(size, PAGE_SIZE);

	sys_free_user_mem(address, size);

	int pages_needed = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	for (int i = first_page; i < first_page + pages_needed; i++) {
		Pages_arr_user_heap[i].allocated = 0;
	}

	return;
}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
