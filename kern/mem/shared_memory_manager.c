#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>

#include <inc/environment_definitions.h>
#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()

	unsigned int totSz = numOfFrames * sizeof(struct FrameInfo*);
	struct FrameInfo** all_frames = (struct FrameInfo**)kmalloc(totSz);
	if (all_frames == NULL) return NULL;

	for (int i = 0; i < numOfFrames; i++) all_frames[i] = NULL;
	return all_frames;

}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()

	struct Share* obj = (struct Share*)kmalloc(sizeof(struct Share));
	if(obj == NULL) return NULL;
    int num_of_page =ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	obj->framesStorage = create_frames_storage(num_of_page);
	if (obj->framesStorage == NULL)
	{
		kfree(obj);
		return NULL;
	}

	strncpy(obj->name, shareName, 64);
	obj->ID = (uint32)((unsigned long)obj & 0x7FFFFFFF),
	obj->ownerID = ownerID,
	obj->size = (int)size,
    obj->isWritable = isWritable,
	obj->references = 1;

	return obj;
}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	acquire_spinlock(&(AllShares.shareslock));
	//bool lock_is_held = holding_spinlock(&AllShares.shareslock);

	struct Share* tmp = AllShares.shares_list.lh_first;
	while(tmp != NULL)
	{
		if(tmp->ownerID == ownerID && strcmp(tmp->name, name) == 0)
			{
			release_spinlock(&(AllShares.shareslock));
			return tmp;
			}
		tmp = tmp->prev_next_info.le_next;
	}
	release_spinlock(&(AllShares.shareslock));

	return NULL;
}
//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment
	if (get_share(ownerID,shareName)!=NULL)
	{     return E_SHARED_MEM_EXISTS;
		}
	struct Share *sharedobj;
	sharedobj=create_share(ownerID,shareName,size,isWritable);

	uint32 Rsize=ROUNDUP(size,PAGE_SIZE);
	uint32 Rva =ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
    uint32 address=Rva;
	int start_page=Rva/PAGE_SIZE;
	int num_of_pgs=Rsize/PAGE_SIZE;
    int perm=0 ;


	if(sharedobj==NULL){
		return E_NO_SHARE;
	}

	if(sharedobj->isWritable==1){
		perm=PERM_WRITEABLE;
	}

	 acquire_spinlock(&(AllShares.shareslock));
	 LIST_INSERT_TAIL(&AllShares.shares_list, sharedobj);
	 release_spinlock(&(AllShares.shareslock));
	 int map_res;

	for (uint32 i = 0; i < num_of_pgs; i++){
		 struct FrameInfo *frame;
          int all=allocate_frame(&frame);

         map_res=map_frame(myenv->env_page_directory, frame, Rva, PERM_PRESENT |PERM_WRITEABLE|PERM_AVAILABLE);

         if ( map_res != 0) {
             for (int R = 0; R < i; R++) {

                 unmap_frame(myenv->env_page_directory, address);
                 sharedobj->framesStorage[R] = NULL;
                 address += PAGE_SIZE;
             }
             return E_NO_MEM;
         }

     sharedobj->framesStorage[i]=frame;
     Rva += PAGE_SIZE;
	}

	return sharedobj->ID;
}




//======================
// [5] Get Share Object:
//======================

int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...
	uint32 va=(uint32)virtual_address;
    struct Share *sharedobj;
	struct Env* myenv = get_cpu_proc(); //The calling environment
	sharedobj=get_share(ownerID,shareName);
	if(sharedobj==NULL)
			return E_SHARED_MEM_NOT_EXISTS ;
	uint32 size=ROUNDUP(sharedobj->size,PAGE_SIZE);
	int num_of_pgs=size/PAGE_SIZE;
	int map_res;
	int perm=0;
	if(sharedobj->isWritable==1){
		perm=PERM_WRITEABLE;
	}
    for(int i=0;i<num_of_pgs;i++){

		map_res=map_frame(myenv->env_page_directory,sharedobj->framesStorage[i],va,PERM_PRESENT|PERM_USER|perm|PERM_AVAILABLE);
		va+=PAGE_SIZE;

	}
    sharedobj->references++;

    return sharedobj->ID;

}
//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself

void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("free_share is not implemented yet");
	//Your Code is Here...

	  acquire_spinlock(&(AllShares.shareslock));
	  LIST_REMOVE(&AllShares.shares_list, ptrShare);
	  release_spinlock(&(AllShares.shareslock));
	   int num_of_frames = ROUNDDOWN(ptrShare->size, PAGE_SIZE) / PAGE_SIZE;

	    if (ptrShare->framesStorage != NULL)
	    {
	        for (int i = 0; i < num_of_frames; i++)
	        {
	            if (ptrShare->framesStorage[i] != NULL)
	            {

	            	 ptrShare->framesStorage[i]->references--;
	            	 if (ptrShare->framesStorage[i]->references == 0) {
	            	             free_frame(ptrShare->framesStorage[i]);
	            	         }
	                 ptrShare->framesStorage[i] = NULL;

	            }
	        }

	        kfree(ptrShare->framesStorage);
	    }

	    kfree(ptrShare);
}


//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("freeSharedObject is not implemented yet");
	//Your Code is Here...
	 struct Env *myenv=  get_cpu_proc();

	    uint32 *ptr_page_table=NULL ;
		uint32 pagetblindex=PTX((uint32)startVA);

		acquire_spinlock(&(AllShares.shareslock));
			struct Share* obj = AllShares.shares_list.lh_first;

			while(obj != NULL)
			{

				if(obj->ID == sharedObjectID )
					{
					break;
					}
				obj = obj->prev_next_info.le_next;

		    }
			 if (obj == NULL) {
			        release_spinlock(&(AllShares.shareslock));
			        return -1;
			    }
       release_spinlock(&(AllShares.shareslock));

		uint32 Rva = ROUNDDOWN((uint32)startVA, PAGE_SIZE);
	    uint32 Rsize = ROUNDUP(obj->size, PAGE_SIZE);
	    uint32 end_va=Rsize+Rva;

		  for(uint32 i=Rva;i <end_va;i+=PAGE_SIZE){
		  		unmap_frame(myenv->env_page_directory,i);
                env_page_ws_invalidate(myenv, i);

	      get_page_table(myenv->env_page_directory,i,&ptr_page_table);
	      if (ptr_page_table != NULL){
	  	    bool page_table_empty = 1;

	    	for(int i=0;i<NPDENTRIES;i++){
	          if (ptr_page_table[i] & PERM_PRESENT) {
	    		   page_table_empty =0 ;
	    		    break;
	    				          }
	    			  }
		  if (page_table_empty) {
		          kfree(ptr_page_table);
                  myenv->env_page_directory[PDX(i)] = 0;

		      }
		  }
          tlb_invalidate(myenv->env_page_directory, (void *)i);

	}
		  obj->references--;

		 if ( obj->references==0)
		 {
			 free_share(obj);
		 }

		return 0;
}
