/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
uint32 *BEG_BLOCK;
uint32 *END_BLOCK;
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...

	BEG_BLOCK = (uint32 *)daStart;
	*BEG_BLOCK = 1;

	END_BLOCK = (uint32 *)(daStart + initSizeOfAllocatedSpace - sizeof(int));
	*END_BLOCK = 1;

	struct BlockElement *firstBlock = (struct BlockElement *) (daStart + 2 * sizeof(int));
	//Set Header and Footer of the FirstBlock
	set_block_data(firstBlock,(uint32 )(initSizeOfAllocatedSpace - 2 * sizeof(int)),0);

	LIST_INIT(&freeBlocksList);
	LIST_INSERT_HEAD(&freeBlocksList, firstBlock);
	cprintf("elements after initialization");
	print_blocks_list(freeBlocksList);


}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("set_block_data is not implemented yet");


	//Your Code is Here...
	uint32 *header = (uint32*)((char*)va - 4);
	uint32 *footer = (uint32*)((char*)va + totalSize - 8) ;
		if(isAllocated == 0){
			*header =   totalSize | 0  ;
			*footer =   totalSize | 0  ;
		}
		else{
			*header =   totalSize | 1  ;
			*footer =   totalSize | 1  ;
		}
//		   cprintf("Header Value: %u\n", *header);
//		   cprintf("Footer Value: %u\n", *footer);
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	if (size == 0) {
	        return NULL;
	    }

	    if (size % 2 != 0) size++;
	    if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
	        size = DYN_ALLOC_MIN_BLOCK_SIZE;


	    if (!is_initialized) {
	        uint32 required_size = size + 2 * sizeof(int) /*header & footer*/ + 2 * sizeof(int) /*da begin & end*/;
	        uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE);
	        uint32 da_break = (uint32)sbrk(0);
	        initialize_dynamic_allocator(da_start, da_break - da_start);
	    }

	    uint32 total_size = size + 2 * sizeof(int);
	    struct BlockElement *cur;

	    void *ra;

	    LIST_FOREACH(cur, &freeBlocksList) {
	        uint32 cur_size = get_block_size(cur);
	        if (cur_size >= total_size) {

	            uint32 diff = cur_size - total_size;
	            ra = (void *)cur;

	            if (diff < 4 * sizeof(int)) {
	                set_block_data(ra, cur_size, 1);
	                LIST_REMOVE(&freeBlocksList, cur);

	                return (void *)(ra);
	            }
	            else {
	                set_block_data(ra, total_size, 1);

	                void *nextVa = ra + total_size;

	                set_block_data(nextVa, diff, 0);

	                LIST_INSERT_AFTER(&freeBlocksList, cur, (struct BlockElement *)nextVa);
	                LIST_REMOVE(&freeBlocksList, cur);
	                return (void *)(ra);
	            }
	        }
	    }

	  return NULL;


}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	if (size == 0)
	{
		return NULL;
	}

	if (size % 2 != 0) size++;
	if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
		size = DYN_ALLOC_MIN_BLOCK_SIZE;


	if (!is_initialized) {
		uint32 required_size = size + 2 * sizeof(int) /*header & footer*/ + 2 * sizeof(int) /*da begin & end*/;
		uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE);
		uint32 da_break = (uint32)sbrk(0);
		initialize_dynamic_allocator(da_start, da_break - da_start);
	}

	struct BlockElement *min_sz_block = NULL, *curr_block = NULL;
	uint32 total_sz = size + 2 * sizeof(int), min_sz = (uint32)-1;

	LIST_FOREACH(curr_block, &freeBlocksList)
	{
		uint32 curr_block_sz = get_block_size(curr_block);

		if(curr_block_sz < min_sz && curr_block_sz >= total_sz)
		{
			min_sz_block = curr_block, min_sz = curr_block_sz;
		}
	}

	if(!min_sz_block)
	{
		return NULL;
	}

	void *ra = (void *)min_sz_block;
	uint32 remaining_space = min_sz - total_sz;
	if(remaining_space <= 4 * sizeof(int))
	{
		set_block_data(ra, min_sz, 1);
	}
	else
	{
		set_block_data(ra, total_sz, 1);

		void *nextVa = ra + total_sz;
		set_block_data(nextVa, remaining_space, 0);

		LIST_INSERT_AFTER(&freeBlocksList, min_sz_block, (struct BlockElement *)nextVa);
	}

	LIST_REMOVE(&freeBlocksList, min_sz_block);

	return ra;
}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("free_block is not implemented yet");
//	Your Code is Here...

////////////////////////////////////////////////////////////////////////
	//initialization we might need
		uint32 cur_size = get_block_size(va)-8;
		uint32 *prev_footer = (uint32 *)((char*)va - 8);
		uint32 *next_header = (uint32 *)((char*)va + cur_size +4);
		uint32 prev_size = ((*prev_footer) & ~0x1) - 8;
		uint32 next_size = ((*next_header) & ~0x1) - 8;
		bool prev_is_allocated = (*prev_footer) & 0x1;
		bool next_is_allocated = (*next_header) & 0x1;
		void* prev_va = (void*)((char*)va - 8 - prev_size);
		void* next_va = (void*)((char*)va + cur_size + 8);
///////////////////////////////////////////////////////////////////////
		//exit conditions
	if(va==NULL){return;}
	if(get_block_size(va)==0||is_free_block(va)){return;}
//////////////////////////////////////////////////////////////////////

	struct BlockElement *va_block = (struct BlockElement *)va;
////////////////////////////////////////////////////////////////////////////
	if(prev_is_allocated==1&&next_is_allocated==1)
	{


		set_block_data(va,get_block_size(va),0);


		if(LIST_EMPTY(&freeBlocksList)==1) // if the list is empty
		{

			LIST_INSERT_HEAD(&freeBlocksList,va_block);
			return;
		}

		struct BlockElement *tmp = LIST_FIRST(&freeBlocksList);
		bool found = 0;
		LIST_FOREACH(tmp, &freeBlocksList)
		{

			if((uint32)tmp>(uint32)va)
			{	found=1;
				LIST_INSERT_BEFORE(&freeBlocksList,tmp,va_block);
				return;
			}
		}

		if(found==0){LIST_INSERT_TAIL(&freeBlocksList,va_block); return;}

	}
////////////////////////////////////////////////////////////////////////////////////////
	else if(prev_is_allocated==0&&next_is_allocated==1)
	{

		uint32 size = prev_size + cur_size + 16;
		set_block_data(prev_va,size,0);
		set_block_data(va,0,0);
		return;
	}
/////////////////////////////////////////////////////////////////////////////////////
	else if(prev_is_allocated==1&&next_is_allocated==0)
	{
		set_block_data(next_va,0,0);
		uint32 size = next_size + cur_size + 16;
		set_block_data(va,size,0);
		struct BlockElement *tmp = (struct BlockElement*) va;
		struct BlockElement *tmp2 = (struct BlockElement*) next_va;    //SECOND APPROACH
		LIST_INSERT_BEFORE(&freeBlocksList,tmp2,tmp);
		LIST_REMOVE(&freeBlocksList,tmp2);
//////////////////////////////////////////////////////////////////////////////////////////////
	}

	else if(prev_is_allocated==0&&next_is_allocated==0)
	{

		set_block_data(next_va,0,0);
		set_block_data(va,0,0);
		uint32 size = next_size + cur_size + prev_size + 8+16;
		set_block_data(prev_va,size,0);
		struct BlockElement *tmp = (struct BlockElement*) next_va;    //SECOND APPROACH
		LIST_REMOVE(&freeBlocksList,tmp);


	}
//////////////////////////////////////////////////////////////////////////////////////////////


}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
		//COMMENT THE FOLLOWING LINE BEFORE START CODING
		//panic("realloc_block_FF is not implemented yet");
		//Your Code is Here...
		if (new_size <= 0)
		{
			if(va != NULL)
			{
				free_block(va);
			}
			return NULL;
		}

		if (new_size % 2 != 0) new_size++;

		if (new_size < DYN_ALLOC_MIN_BLOCK_SIZE)
		{
			new_size = DYN_ALLOC_MIN_BLOCK_SIZE;
		}

		if(va == NULL)
		{
			return alloc_block_FF(new_size);
		}

		int size = new_size + 2 * sizeof(int);
		int old_size = get_block_size(va);

		if(size == old_size)
		{
			return va;
		}
		else if(size < old_size)
		{

			set_block_data(va, size, 1);

			void *next_va = (char *)va + old_size;

			int next_size = get_block_size(next_va);

			bool next_is_free = is_free_block(next_va);

			if(next_is_free)
			{
				int next_new_size = old_size-size+next_size;
				void *next_new_va= (char *)va+size;
				set_block_data(next_new_va, next_new_size, 0);
				LIST_INSERT_BEFORE(&freeBlocksList, (struct BlockElement *)next_va, (struct BlockElement *)next_new_va);
				LIST_REMOVE(&freeBlocksList, (struct BlockElement *)next_va);
				return va;
			}
			else
			{
				int next_new_size = old_size - size;
				if(next_new_size < 4 * sizeof(int))
				{
					set_block_data(va, old_size, 1);
					return va;
				}
				else
				{
					set_block_data(va, size, 1);
					void *next_new_va = (char*)va + size;
					set_block_data(next_new_va, old_size - size, 0);

					struct BlockElement *block = LIST_FIRST(&freeBlocksList);

					LIST_FOREACH(block, &freeBlocksList)
					{

						if((uint32)next_new_va < (uint32)block)
						{
							LIST_INSERT_BEFORE(&freeBlocksList, block, (struct BlockElement *)next_new_va);
							return va;
						}
					}

					LIST_INSERT_TAIL(&freeBlocksList, (struct BlockElement *)next_new_va);

					return va;
				}
			}
		}
		else if(size > old_size)
		{

			void *next_va = (char *)va + old_size;

			bool next_is_free = is_free_block(next_va);

			int next_size = get_block_size(next_va);

			int total_size_after_merging = old_size + next_size;

			if(next_is_free && total_size_after_merging >= size)
			{
				if(total_size_after_merging - size < 4 * sizeof(int))
				{
					set_block_data(va, total_size_after_merging, 1);
					LIST_REMOVE(&freeBlocksList, (struct BlockElement *)next_va);
				}
				else
				{
					set_block_data(va, size, 1);
					void *next_new_va = (char *)va + size;
					set_block_data(next_new_va, total_size_after_merging - size, 0);
					LIST_INSERT_AFTER(&freeBlocksList, (struct BlockElement *)next_va, (struct BlockElement *)next_new_va);
					LIST_REMOVE(&freeBlocksList, (struct BlockElement *)next_va);
				}
			}
			else
			{
				free_block(va);
				return alloc_block_FF(new_size);
			}

		}
		return va;
}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
