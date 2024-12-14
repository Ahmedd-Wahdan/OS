// User-level Semaphore

#include "inc/lib.h"


struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	struct semaphore sem;

		sem.semdata = smalloc(semaphoreName, sizeof(struct __semdata), 1);
		sem.semdata->count = value;
		sem.semdata->lock=0;
		strncpy(sem.semdata->name,semaphoreName,64);
		sys_init_queue(&(sem.semdata->queue));


		return sem;

}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	struct __semdata* semdata = (struct __semdata*) sget(ownerEnvID,semaphoreName);
	 struct semaphore sem ;
	 sem.semdata=semdata;
	 return sem;


}

void wait_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...

	while (xchg(&(sem.semdata->lock), 1) != 0);
	sem.semdata->count--;
	if (sem.semdata->count < 0) {
//cprintf("\n before enqueue \n");
		sys_queue((sem.semdata),0);
//		cprintf("\n after enqueue \n");

        return;
	}


		sem.semdata->lock = 0;


}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
//	cprintf("\n hiiiiiiiiiiiiiiii\n");
//	int keyS=1;
	while(xchg(&(sem.semdata->lock), 1) != 0) ;

	sem.semdata->count++;
	if (sem.semdata->count<=0){
		sys_queue((sem.semdata),1);
		return ;

	}
	sem.semdata->lock=0;

}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
