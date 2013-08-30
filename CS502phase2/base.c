/************************************************************************

        This code forms the base of the operating system you will
        build.  It has only the barest rudiments of what you will
        eventually construct; yet it contains the interfaces that
        allow test.c and z502.c to be successfully built together.      

        Revision History:       
        1.0 August 1990
        1.1 December 1990: Portability attempted.
        1.3 July     1992: More Portability enhancements.
                           Add call to sample_code.
        1.4 December 1992: Limit (temporarily) printout in
                           interrupt handler.  More portability.
        2.0 January  2000: A number of small changes.
        2.1 May      2001: Bug fixes and clear STAT_VECTOR
        2.2 July     2002: Make code appropriate for undergrads.
                           Default program start is in test0.
        3.0 August   2004: Modified to support memory mapped IO
        3.1 August   2004: hardware interrupt runs on separate thread
        3.11 August  2004: Support for OS level locking

		Updated by Jiefeng He 
		10.0 Oct.    2012	(nearly 2000 lines of code)
		PS:
		The whole system is runned in Windows 7.
		The code is compiled and tested in Microsoft Visual Studio 2012.
************************************************************************/

#include             "global.h"
#include             "syscalls.h"
#include             "protos.h"
#include             "string.h"
#include			 "my_includes.h"
#include			 <stdlib.h>

extern char          MEMORY[];  
//extern BOOL          POP_THE_STACK;
extern UINT16        *Z502_PAGE_TBL_ADDR;
extern INT16         Z502_PAGE_TBL_LENGTH;
extern INT16         Z502_PROGRAM_COUNTER;
extern INT16         Z502_INTERRUPT_MASK;
extern INT32         SYS_CALL_CALL_TYPE;
extern INT16         Z502_MODE;
extern Z502_ARG      Z502_ARG1;  //defined in z502.c
extern Z502_ARG      Z502_ARG2;
extern Z502_ARG      Z502_ARG3;
extern Z502_ARG      Z502_ARG4;
extern Z502_ARG      Z502_ARG5;
extern Z502_ARG      Z502_ARG6;

extern void          *TO_VECTOR [];
extern INT32         CALLING_ARGC;
extern char          **CALLING_ARGV;

char                 *call_names[] = { "mem_read ", "mem_write",
                            "read_mod ", "get_time ", "sleep    ", 
                            "get_pid  ", "create   ", "term_proc", 
                            "suspend  ", "resume   ", "ch_prior ", 
                            "send     ", "receive  ", "disk_read",
                            "disk_wrt ", "def_sh_ar" };

/*gloabl variables defined by Jiefeng*/
PCB *pcb; //Temple

//Four places where PCBs exist
PCB *current_pcb;
TQ TimerQueue_head;
TQ ReadyQueue_head;
TQ WaitingQueue_head;
DQ DiskQueue_head;
FF FreeFrameList_head;

MQ MessageQueue_head;

UINT16 *memory_organizer[PHYS_MEM_PGS];
UINT16 process_storage[PHYS_MEM_PGS];
BOOL   is_dirty[PHYS_MEM_PGS];
UINT16 *address_storgae[10];

//help decides whether certain PCB exists or not
int process_pid[MAX_PROCESS_NUMBER];
//help decides whether certain pcb is suspended or not
int suspend_flag[MAX_PROCESS_NUMBER];
int count_pcb_id=0;
int message_count=0;
int count=0;
//int reference_index=-1;
int reference_index2=-1;
int memory_organizer_index=-1;
int flag=-1;
int just_for_testing=0;

BOOL FULL_PRINT=TRUE;
BOOL scheduler_full_print=FALSE;
BOOL obvisouly_print=TRUE;

TQ temp,temp1;

void *os_make_process1(void *a,void *b, int c);
PCB *os_make_process(int flag);
PCB *RemoveFromTimerQueue();
int check_process_name(void *a);
void AddToReadyQueue(PCB *pcb);
void AddToTimerQueue(PCB *pcb);
void AddToDiskQueue(INT16 disk_id, int pid);
void AddToFreeFrameList(short frame_number);
void PrintReadyQueue();
void PrintTimerQueue();
void PrintWaitingQueue();
void PrintDiskQueue();
void os_end_process();
void add_child(int a);
void delete_me_process();
PCB *RemoveFromReadyQueue();
DiskQ *RemoveFromDiskQueue();
PCB *RemoveFromWaitingQueueWithDiskID(INT16 disk_id);
PCB *RemoveFromWaitingQueueWithPID(int pid);
short RemoveFromFreeFrameList();
int check_children_process(int a, PCB *pcb);
void delete_process_id_in_parent(int a, PCB *pcb);
void delete_family_process(int a);
void delete_the_process(int a);
void os_get_process_id(void *a);
void dispatcher();
void kill_time();
void os_suspend_process(int pid);
void os_resume_process(int pid);
void AddToWaitingQueue(PCB *pcb_waiting);
void AddBackToWaitingQueue(PCB *pcb_waiting);
PCB *RemoveFromWaitingQueue(int pid);
BOOL IS_IN_TIMERQUEUE(int pid);
BOOL IsSafeNow(short);
void os_change_priority(long pid,long priority1);
void os_send_message(long target_id, void *message, long send_length);
void os_receive_message(long source_id, void *msg_buffer, long receive_length);
void AddToMessageQueue(Message *msg);
void set_printer();
void os_disk_write(INT32 disk_id,INT32 sector, char *buffer);
void os_disk_read(INT32 disk_id,INT32 sector, char *buffer);

/************************************************************************
					OS_CHANGE_PRIORITY
When user calls for change priortiy of a certain process, 
this routine is called by svc to complete the task.
It first checks whether the parameters are good, if not then return 
an error, otherwise search through current_pcb, TimerQueue, WaitingQueue 
and ReadyQueue and find the certain process, changes its priority. When 
the process is in ReadyQueue, we also need to rearrange the ReadyQueue.

In: pid of the process, attributes of the priority
Out: void
***********************************************************************/
void os_change_priority(long pid,long priority1)
{
	*(long *) Z502_ARG3.PTR = ERR_SUCCESS ;

	if (pid<-1||pid>=MAX_PROCESS_NUMBER) //whether voilate the process id
	{
		*(long *) Z502_ARG3.PTR = ERR_NONE_PROCESS_ID ;
	}
	else if (process_pid[(int)pid]==-1) //whether exists such a process
	{
		*(long *) Z502_ARG3.PTR = ERR_NONE_PROCESS_ID ;
	}
	else if (priority1<0||priority1>100) //whether priority is reasonalbe
	{
		*(long *) Z502_ARG3.PTR = ERR_ILLEGAL_PRIORITY;
	}
	else
	{
		//change current pcb
		if (pid==-1||current_pcb->process_id==pid)
		{
			printf("***********PCB%d has changed its priority from %d to %d.**********\n",current_pcb->process_id,current_pcb->priority,priority1);
			current_pcb->priority=(int)priority1;
		}
		else
		{
			//lock TimerQueue, check and try to change it, if done, return
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			if(TimerQueue_head!=NULL)
			{
				temp =TimerQueue_head ;
				if (temp->data->process_id==(int)pid)
				{
					printf("***********PCB%d has changed its priority from %d to %d.**********\n",pid,temp->data->priority,priority1);
					temp->data->priority=(int)priority1;
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
					return;
				}
				while(temp->next!=NULL)
				{
					temp=temp->next;
					if (temp->data->process_id==(int)pid)
					{
						printf("***********PCB%d has changed its priority from %d to %d.**********\n",pid,temp->data->priority,priority1);
						temp->data->priority=(int)priority1;
						Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
						return;
					}
				}
			}
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);

			//lock WaitingQueue, check and try to change it, if done, return
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			if(WaitingQueue_head!=NULL)
			{
				temp =WaitingQueue_head ;
				if (temp->data->process_id==pid)
				{
					printf("***********PCB%d has changed its priority from %d to %d.**********\n",pid,temp->data->priority,priority1);
					temp->data->priority=priority1;
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
					return;
				}
				while(temp->next!=NULL)
				{
					temp=temp->next;
					if (temp->data->process_id==pid)
					{
						printf("***********PCB%d has changed its priority from %d to %d.**********\n",pid,temp->data->priority,priority1);
						temp->data->priority=priority1;
						Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
						return;
					}
				}
			}
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);

			//lock ReadyQueue, check and try to change it, if done, return
			//In ReadyQueue after changing the priority, we need to rearrange the order of the ReadyQueue
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			if(ReadyQueue_head!=NULL)
			{
				TQ temp3;
				temp =ReadyQueue_head ;
				temp1=temp;
				if (temp->data->process_id==(int)pid)
				{
					printf("***********PCB%d has changed its priority from %d to %d.**********\n",pid,temp->data->priority,priority1);
					temp->data->priority=(int)priority1;
					if (temp1->next!=NULL)
					{
						temp1=temp1->next;
						temp3=temp1;
						while ((temp1->next!=NULL)&&(temp1->data->priority<=temp->data->priority))
						{
							temp3=temp1;
							temp1=temp1->next;
						}
						//situation 1: no need to change
						if (temp3==temp1&&temp1->data->priority>temp->data->priority)
						{
							;
						}
						//situation 2: need to add to the end
						else if (temp1->data->priority<=temp->data->priority)
						{
							ReadyQueue_head=temp->next;
							temp->next=NULL;
							temp1->next=temp;
						}
						//situation 3: need to add to between temp3 and temp1
						else
						{
							ReadyQueue_head=temp->next;
							temp->next=temp1;
							temp3->next=temp;
						}
					}
				}
				else
				{
					while(temp->next!=NULL)
					{
						temp1=temp;
						temp=temp->next;
						if (temp->data->process_id==(int)pid)
						{
							printf("***********PCB%d has changed its priority from %d to %d.**********\n",pid,temp->data->priority,priority1);
							temp->data->priority=(int)priority1;
							temp1->next=temp->next;//cut the temp
							AddToReadyQueue(temp->data);
							break;
						}
					}

				}
			}
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		}
	}
}

/***********************************************************************
							OS_SUSPEND_PROCESS
When user calls for suspend a certain process, this routine is called by 
svc to complete the task. 
It first checks whether the parameters are good, if not then return 
an error, otherwise search through TimerQueue and ReadyQueue and find the
certain process. If it is in TimerQueue, then done nothing just make suspend_flag
to be 0, we will deal with this later in interrupt_handler. If it is in ReadyQueue
, we put it to the WaitingQueue and change suspend_flag.

In: pid of to be suspended process
Out: void
**********************************************************************/
void os_suspend_process(int pid)
{
	*(long *)Z502_ARG2.PTR = ERR_SUCCESS;
	if (pid<-1||pid>=MAX_PROCESS_NUMBER) //whether voilate the process id
	{
		*(long *)Z502_ARG2.PTR = ERR_NONE_PROCESS_ID;
	}
	else if (process_pid[pid]==-1&&pid!=-1) //whether exists such a process
	{
		*(long *)Z502_ARG2.PTR = ERR_NONE_PROCESS_ID;
	}
	else if ((pid==-1)||(current_pcb->process_id==pid)) //not permitted to suspend oneself
	{
		*(long *)Z502_ARG2.PTR = ERR_SUSPEND_ONESELF;
	}
	else
	{
		if (suspend_flag[pid]==-1) 	//not suspend yet
		{
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			if (!IS_IN_TIMERQUEUE(pid))
			{
				//NOT in TimerQueue means it must be in ReadyQueue
				temp=ReadyQueue_head;
				temp1=temp;
				while (temp->data->process_id!=pid)
				{
					temp1=temp;
					temp=temp->next;
				}
				//suspend the first
				if (temp1==temp)
				{
					ReadyQueue_head=temp->next;
					temp->next=NULL;
				}
				else
				{
					temp1->next=temp->next;
					temp->next=NULL;
				}
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
				AddToWaitingQueue(temp->data);
				if (!FULL_PRINT) //print format as jb asked
				{
				CALL( SP_setup_action( SP_ACTION_MODE, "Suspend" ) );
				CALL( SP_setup( SP_TARGET_MODE, (INT32)temp->data->process_id ) );
				CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
				set_printer();
				if ((count++)%15==0)
				{
					CALL( SP_print_header() );
				} 
				CALL( SP_print_line() );
				}
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			}
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
			suspend_flag[pid]=0; //change the flag
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		}
		else //the process has already been suspended
		{
			*(long *)Z502_ARG2.PTR = ERR_ALREADY_SUSPENDED;
		}
	}
}

/********************************************************************
				OS_RESUME_PROCESS
When user calls for resume a certain process, this routine is called by 
svc to complete the task. 
It first checks whether the parameters are good, if not then return 
an error, otherwise search through TimerQueue and WaitingQueue and find the
certain process. If it is in TimerQueue, then done nothing just make suspend_flag
to be -1. If it is in WaitingQueue, we put it to the ReadyQueue and change suspend_flag.

In: pid of to be resumed process
Out: void
******************************************************************/
void os_resume_process(int pid)
{
	*(long *)Z502_ARG2.PTR = ERR_SUCCESS;
	if (pid<-1||pid>=MAX_PROCESS_NUMBER) //whether voilate the process id
	{
		*(long *)Z502_ARG2.PTR = ERR_NONE_PROCESS_ID;
	}
	else if (process_pid[pid]==-1&&pid!=-1) //whether exists such a process
	{
		*(long *)Z502_ARG2.PTR = ERR_NONE_PROCESS_ID;
	}
	else if ((pid==-1)||(current_pcb->process_id==pid)) //no suspend oneself means no resume oneself
	{
		*(long *)Z502_ARG2.PTR = ERR_RESUME_ONESELF;
	}
	else
	{
		//since only in main thread will change the suspend_flag so no need to lock here,
		//otherwise will easily lead to deadlock
		if (suspend_flag[pid]==0) //already suspended
		{
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			if (!IS_IN_TIMERQUEUE(pid))
			{
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
				pcb=RemoveFromWaitingQueue(pid);
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
				AddToReadyQueue(pcb);
				if (!FULL_PRINT) //print format as jb asked
				{
				CALL( SP_setup_action( SP_ACTION_MODE, "Resume" ) );
				CALL( SP_setup( SP_TARGET_MODE, (INT32)pid ) );
				CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
				set_printer();
				if ((count++)%15==0)
				{
					CALL( SP_print_header() );
				} 
				CALL( SP_print_line() );
				}
			}
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
			suspend_flag[pid]=-1; //change the flage to show that it's not suspended any more
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		}
		else //Not suspended now
		{
			*(long *)Z502_ARG2.PTR = ERR_ALREADY_RESUMED;
		}
	}
}

/*****************************************************
					KILL_TIME
Called by dispatcher routine when we need some processes 
to be in the ReadyQueue.
Just Do nothing.

In:void
Out:void
*****************************************************/
void kill_time()
{
	;
}

/*****************************************************
					DISPATCHER
When some process fall asleep, we need the dispatcher to deal with
this situation to make the other process in ready to run.
First check whether ReadyQueue is empty, if not find the first in
ReadyQueue to run. Otherwise, call a routine to kill time and 
wait for some process to be ready and then run it.

In:void
Out:void
*****************************************************/
void dispatcher()
{
	//using call will spend some time while using zcall won't
	//we cannot lock here, otherwise will lead to deadlock
	INT32 Time;
	PCB *current_pcb_temp;
	static int  k=0;
	
	if (just_for_testing)
	{
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		PrintWaitingQueue();
		PrintReadyQueue();
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
	}
	
	//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
	//PrintDiskQueue();
	//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);

	while (ReadyQueue_head==NULL)
	{

		CALL(kill_time());//if no process is ready, just wait
		//ZCALL( MEM_READ(Z502ClockStatus, &Time) );
		//if(0==0)
			//printf("%d\n",Time);
		//PrintTimerQueue();
	}

	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
	if (obvisouly_print)
	{
		if (scheduler_full_print||k<20)
		{                    //print format as jb asked
			CALL( SP_setup_action( SP_ACTION_MODE, "Dispatch" ) );
			CALL( SP_setup( SP_TARGET_MODE, (INT32)ReadyQueue_head->data->process_id ) );
			CALL( SP_setup( SP_RUNNING_MODE, (INT32)ReadyQueue_head->data->process_id ) );
			set_printer();
			if ((count++)%15==0)
			{
				CALL( SP_print_header() );
			} 
			CALL( SP_print_line() );
			k++;
		}
	}
	current_pcb_temp=RemoveFromReadyQueue();
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);

	if (current_pcb_temp!=current_pcb)
	{
		//printf("switch from %d to %d\n",current_pcb->process_id,current_pcb_temp->process_id);
		current_pcb=current_pcb_temp;
		ZCALL( Z502_SWITCH_CONTEXT( SWITCH_CONTEXT_SAVE_MODE, &(current_pcb->context) ) );
		// i don't know why, but fortunately found this way to solve this problem
		// if just put switch_context here, then in Z502.C hardwarelock will get stucked
		//at the same time i'm wondering if the code in z502.c is wrong
		//maybe it should be GetLock( InterruptLock, "hardware_interrupt-2" ); in stead of GetLock( Hardwarelock, "hardware_interrupt-2" );
		//anyway, since the problem is solved, i don't care these trivial things :)
	}

	return;
}

/********************************************************************************
	OS_GET_PROCESS_ID   
When user calls for get the id of a certain process, this routine is called by 
svc to complete the task. 
It checks through the current_pcb, TimerQueue, ReadyQueue and WaitingQueue
to find the id of the process having certain names. If not find, an Error is reported.

In: the name of a process
Out: void (In fact registers have been changed)
*******************************************************************************/
void os_get_process_id(void *a)
{
	*(long *) Z502_ARG3.PTR = ERR_SUCCESS ;
	*(long *) Z502_ARG2.PTR = -1L ;
	
	if (strcmp("",(const char *)a)==0)
	{
		*(long *) Z502_ARG2.PTR =(long) current_pcb->process_id;
		return;
	}

	//check current_pcb
	if (strcmp(current_pcb->process_name,(const char *)a)==0)
	{
			*(long *) Z502_ARG2.PTR =(long) current_pcb->process_id;
			return;
	}
	
	//check TimerQueue
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
	if(TimerQueue_head!=NULL)
	{
		temp =TimerQueue_head ;
		if (strcmp(temp->data->process_name,(const char *)a)==0)
		{
			*(long *) Z502_ARG2.PTR =(long) temp->data->process_id;
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			return;
		}
		while(temp->next!=NULL)
		{
			temp=temp->next;
			if (strcmp(temp->data->process_name,(const char *)a)==0)
			{
				*(long *) Z502_ARG2.PTR =(long) temp->data->process_id;
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
				return;
			}
		}
	}
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);

	//check ReadyQueue
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
	if(ReadyQueue_head!=NULL)
	{
		temp =ReadyQueue_head ;
		if (strcmp(temp->data->process_name,(const char *)a)==0)
		{
			*(long *) Z502_ARG2.PTR =(long) temp->data->process_id;
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			return;
		}
		while(temp->next!=NULL)
		{
			temp=temp->next;
			if (strcmp(temp->data->process_name,(const char *)a)==0)
			{
				*(long *) Z502_ARG2.PTR =(long) temp->data->process_id;
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
				return;
			}
		}
	}
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);

	//check WaitingQueue
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
	if(WaitingQueue_head!=NULL)
	{
		temp =WaitingQueue_head ;
		if (strcmp(temp->data->process_name,(const char *)a)==0)
		{
			*(long *) Z502_ARG2.PTR =(long) temp->data->process_id;
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			return;
		}
		while(temp->next!=NULL)
		{
			temp=temp->next;
			if (strcmp(temp->data->process_name,(const char *)a)==0)
			{
				*(long *) Z502_ARG2.PTR =(long) temp->data->process_id;
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
				return;
			}
		}
	}
	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);

	*(long *) Z502_ARG3.PTR = ERR_NONE_PROCESS_ID ;
}

/**********************************************************************
		DELETE_THE_PROCESS
Delete a process with certain pid.

In: pid of a process
Out: void
************************************************************************/
void delete_the_process(int a)
{
	process_pid[a]=-1;
	//如果是TimerQueue的第一个 还有很麻烦的事情要处理
	//如果是TimerQueue的后面几个 应该可以直接删除
	//In ReadyQueue
	if(ReadyQueue_head!=NULL)
	{
		if (ReadyQueue_head->data->process_id==a)
		{
			if (ReadyQueue_head->next==NULL)
			{
				ReadyQueue_head=NULL;
			}
			else
			{
				ReadyQueue_head=ReadyQueue_head->next;
			}
		}	
		else
		{
			temp=ReadyQueue_head;

			while(temp->next!=NULL)
			{
				temp1=temp;	
				temp=temp->next;
				if (temp->data->process_id==a)
				{
					temp1->next=temp->next;
				}
			}
		}
	}
}

/*********************************************************************************
					DELETE_FAMILY_PROCESS
An issue that jb said I don't need to care about. So the code is actually discarded
and some bugs in it.
**********************************************************************************/
void delete_family_process(int a)
{
	struct offspring *temp_offspring;

	//if current_pcb
	if (a==current_pcb->process_id)
	{
		if(current_pcb->children!=NULL)
		{
			//直接一次删了就是 别管结构重置了		
			temp_offspring=current_pcb->children;
			delete_family_process(temp_offspring->child_id);
			process_pid[temp_offspring->child_id]=-1;
			while(temp_offspring->next_child!=NULL)
			{
				temp_offspring=temp_offspring->next_child;
				delete_family_process(temp_offspring->child_id);
				process_pid[temp_offspring->child_id]=-1;
			}
			current_pcb->children=NULL;
		}
			delete_me_process();
	}
	else
	{
		if (ReadyQueue_head!=NULL)
		{
			temp =ReadyQueue_head;
			if (temp->data->process_id==a)
			{
				if (temp->data->children!=NULL)
				{
					temp_offspring=temp->data->children;
					delete_family_process(temp_offspring->child_id);
					process_pid[temp_offspring->child_id]=-1;
					while(temp_offspring->next_child!=NULL)
					{
						temp_offspring=temp_offspring->next_child;
						delete_family_process(temp_offspring->child_id);
						process_pid[temp_offspring->child_id]=-1;
					}
					temp->data->children=NULL;
				}
				delete_the_process(a);//Although simple, there are redundance here
			}
			else
			{
				while(temp->next!=NULL)
				{
					temp=temp->next;
					if (temp->data->process_id==a)
					{
						if (temp->data->children!=NULL)
						{
							temp_offspring=temp->data->children;
							delete_family_process(temp_offspring->child_id);
							process_pid[temp_offspring->child_id]=-1;
							while(temp_offspring->next_child!=NULL)
							{
								temp_offspring=temp_offspring->next_child;
								delete_family_process(temp_offspring->child_id);
								process_pid[temp_offspring->child_id]=-1;
							}
							temp->data->children=NULL;
						}
						delete_the_process(a);//Although simple, there are redundance here
					}
					break;
				}
			}
		}
		else//在TimerQueue中
		{

		}
	}
}

/*************************************************************************
						IS_IN_TIMERQUEUE
To check whecher the process having the certain id exits in TimerQueue.
Just search the linked list.

In: pid of a process
Out: Bool 
*************************************************************************/
BOOL IS_IN_TIMERQUEUE(int pid)
{
	BOOL flag=FALSE;
	if(TimerQueue_head!=NULL)
	{
		temp =TimerQueue_head ;
		if (temp->data->process_id==pid)
		{
			flag=TRUE;
		}
		else
		{
			while(temp->next!=NULL)
			{
				temp=temp->next;
				if (temp->data->process_id==pid)
				{
					flag=TRUE;
				}
			}
		}
	}
	return flag;
}

/************************************************************************
							CHECK_PROCESS_NAME
Used in os_make_process1 routine to check whether a certain name has existed.
Check curent process, process in TimerQueue, ReadyQueue and WaitingQueue.
If exist the same name, return 0; else return 1	.

In: the name of a process
Out: int between 0 and 1
************************************************************************/
int check_process_name(void *a)
{
	//check current pcb
	TQ temp;
	if (strcmp(current_pcb->process_name,(const char *)a)==0)
		{
			//printf("current_pcb conflict!\n");
			return 0;
	}
	
	//check TimerQueue
	if(TimerQueue_head!=NULL)
		{
			temp =TimerQueue_head ;
			if (strcmp(temp->data->process_name,(const char *)a)==0)
			{
				//printf("TimerQueue head conflict!\n");
				return 0;
			}
			while(temp->next!=NULL)
				{
					temp=temp->next;
					if (strcmp(temp->data->process_name,(const char *)a)==0)
					{
						//printf("TimerQueue body conflict!\n");
						return 0;
					}
			}
	}
	//check ReadyQueue
	if(ReadyQueue_head!=NULL)
		{
			temp =ReadyQueue_head ;
			if (strcmp(temp->data->process_name,(const char *)a)==0)
			{
				return 0;
			}
			while(temp->next!=NULL)
				{
					temp=temp->next;
					if (strcmp(temp->data->process_name,(const char *)a)==0)
					{
						return 0;
					}
			}
	}
	//check WaitingQueue
	if(WaitingQueue_head!=NULL)
		{
			temp =WaitingQueue_head ;
			if (strcmp(temp->data->process_name,(const char *)a)==0)
			{
				return 0;
			}
			while(temp->next!=NULL)
				{
					temp=temp->next;
					if (strcmp(temp->data->process_name,(const char *)a)==0)
					{
						return 0;
					}
			}
	}
	return 1;
}

/**************************************************************************
		CHECK_CHILDREN_PROCESS
To check whether a process is the children of another process.
If exists return 0; else 1. Howerver, this is desinged to help terminate process
which is not use in this architechure of this operating system as jb suggested.
**************************************************************************/
int check_children_process(int a, PCB *pcb)
{
	if (pcb->children==NULL)
	{
		return 1;
	}
	else
	{
		struct offspring *temp;
		//temp=(struct offspring *)malloc(sizeof(struct offspring ));
		temp =pcb->children;
		if (pcb->children->child_id==a)
				return 0;
		while(temp->next_child!=NULL)
			{
				temp=temp->next_child;
				if (pcb->children->child_id==a)
					return 0;
			}
		return 1;
	}
}

/******************************************************************
					DELETE_PROCESS_ID_IN_PARENT
When a process is deleted, delete the mark in its parent.
A routine desinged to help terminate process which is not use in this
architechure of this operating system as jb suggested.
******************************************************************/
void delete_process_id_in_parent(int a, PCB *pcb)
{
	struct offspring *temp,*temp1;
	//temp=(struct offspring *)malloc(sizeof(struct offspring ));
	temp =pcb->children;
	if (pcb->children->child_id==a)
	{
		if (pcb->children->next_child==NULL)
		{
			pcb->children=NULL;
		}
		else
		{
			pcb->children=pcb->children->next_child;
		}
	}	
	else
	{
		while(temp->next_child!=NULL)
		{
			temp1=temp;	
			temp=temp->next_child;
			if (temp->child_id==a)
			{
				temp1->next_child=temp->next_child;
			}
		}
	}
}

/************************************************************************
			 PrintTimerQueue
Just for debugging
************************************************************************/
void PrintTimerQueue()
	{
		//printf ("                   Welcome to PrintTimerQueue!*********************\n");
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
		if (TimerQueue_head==NULL)
			printf("No Process Control Block stored in TimerQueue!\n");
		else
			{
				//TQ temp;  changed to a global variable
				temp = TimerQueue_head ;
				//printf ("We have PCB ID:%d Name:%s Wake_up_time:%ld\n",temp->data->process_id,temp->data->process_name,temp->data->wake_up_time);
				while(temp ->next != NULL )
				{
					temp =temp ->next ;
					//printf ("We have PCB ID:%d Name:%s Wake_up_time:%ld\n",temp->data->process_id,temp->data->process_name,temp->data->wake_up_time);
				}
		}
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
}

/************************************************************************
    PrintReadyQueue
Just for debugging
************************************************************************/
void PrintReadyQueue()
	{
		//printf ("***********Welcome to PrintReadyQueue!*********************\n");
		if (ReadyQueue_head==NULL)
			printf("No Process Control Block stored in ReadyQueue!\n");
		else
			{
				//TQ temp;  changed to a global variable
				temp = ReadyQueue_head ;
				printf ("ReadyQueue: ID:%d %s    ",temp->data->process_id,temp->data->process_name);
				while(temp ->next != NULL )
				{
					temp =temp ->next ;
					printf ("PCB ID:%d %s    ",temp->data->process_id,temp->data->process_name);
				}
				printf("\n");
		}
}

/************************************************************************
    PrintWaitingQueue
Just for debugging
************************************************************************/
void PrintWaitingQueue()
{
		if (WaitingQueue_head==NULL)
			printf("No Process Control Block stored in WaitingQueue!\n");
		else
		{
				temp = WaitingQueue_head ;
				printf ("WaitingQueue ID:%d-%s-%d  ",temp->data->process_id,temp->data->process_name,temp->data->step_index);
				while(temp->next != NULL )
				{
					temp =temp ->next ;
					printf ("ID:%d-%s-%d  ",temp->data->process_id,temp->data->process_name,temp->data->step_index);
				}
				printf("\n");
		}
}

/************************************************************************
    PrintDiskQueue
Just for debugging
************************************************************************/
void PrintDiskQueue()
{
		if (DiskQueue_head==NULL)
			printf("No Process Control Block stored in DiskQueue!\n");
		else
		{
			DQ temp;
			temp = DiskQueue_head ;
			printf ("disk_id:%d pid:%d     ",temp->data->disk_id,temp->data->pid);
			while(temp->next != NULL )
			{
				temp =temp ->next ;
				printf ("disk_id:%d pid:%d     ",temp->data->disk_id,temp->data->pid);
			}
			printf("\n");
		}
}

/************************************************************************
					  RemoveFromTimerQueue
To remove a process out of TimerQueue. Used by interrupt_handler.
If TimerQueue is empty, report an error, otherwise extract the first process
in the TimerQueue.

In: void
Out: a PCB
************************************************************************/
PCB *RemoveFromTimerQueue()
	{
	//if TimerQueue is empty
		if (TimerQueue_head==NULL) //TimerQueue=NULL a huge mistake
			{
				printf("No Process Control Block stored in TimerQueue!\n");
				return NULL;
		}
	//if TimerQueue is not empty
		else 
		{			
			temp=TimerQueue_head;
			TimerQueue_head = temp ->next ;
			return temp->data;
		}
}

/*
developed version
not in use finally
*/
DiskQ *RemoveFromDiskQueue()
	{
	//if DiskQueue is empty
		if (DiskQueue_head==NULL) //DiskQueue=NULL a huge mistake
			{
				printf("Nothing stored in DiskQueue!\n");
				return NULL;
		}
	//if DiskQueue is not empty
		else 
		{			
			DQ temp;
			temp=DiskQueue_head;
			DiskQueue_head = temp ->next ;
			return temp->data;
		}
}

/*
	RemoveFromFreeFrameList
	Each time tries to get one free frame from a list which constructed in os_init
	if get one, return the number of this free frame
	if no free frame any more, return -1
*/
short RemoveFromFreeFrameList()
{
	FF temp;
	if (FreeFrameList_head==NULL)
	{
		return -1;
	}
	
	temp=FreeFrameList_head;
	FreeFrameList_head=FreeFrameList_head->next;
	temp->next=NULL;
	return temp->frame_number;
}

/************************************************************************
							  RemoveFromReadyQueue
To remove a process out of ReadyQueue. Used by dispatcher.
If ReadyQueue is empty, report an error, otherwise extract the first process
in the ReadyQueue.

In: void
Out: a PCB
************************************************************************/
PCB *RemoveFromReadyQueue()
	{
		if (ReadyQueue_head==NULL)
		{
			printf("No Process Control Block stored in ReadyQueue!\n");
			return NULL;
		}
		else 
		{			
			temp=ReadyQueue_head;
			ReadyQueue_head=temp->next ;
			return temp->data;
		}
}

/************************************************************************
							  RemoveFromWaitingQueue
To remove a certain process out of WaitingQueue. Used by os_resume_process.
Check the WaitingQueue with a pid, find and extract the process of out WaitingQueue

In: pid of a process
Out: a PCB
************************************************************************/
PCB *RemoveFromWaitingQueue(int pid)
{
	temp=WaitingQueue_head;
	temp1=temp;
	while (temp->data->process_id!=pid)
	{
		temp1=temp;
		temp=temp->next;
	}
	//remove the first
	if (temp1==temp)
	{
		WaitingQueue_head=temp->next;
		temp->next=NULL;
	}
	else //remove in the middle
	{
		temp1->next=temp->next;
		temp->next=NULL;
	}
	return temp->data;
}

/*
	RemoveFromWaitingQueueWithDiskID
	tries to remove the fisrt pcb in the watiting queue that share the same disk id as parameter disk_id
	just go through the waiting queue list
	if finde one, return the pcb
	if not, return NULL
*/
PCB *RemoveFromWaitingQueueWithDiskID(INT16 disk_id)
{
	TQ temp,temp1;
	
	if (WaitingQueue_head==NULL)
	{
		return NULL;
	}

	temp=WaitingQueue_head;
	temp1=temp;
	while (temp->data->disk_id_index!=disk_id)
	{
		temp1=temp;
		temp=temp->next;
		if (temp==NULL)
			return NULL;
	}
	//remove the first
	if (temp1==temp)
	{
		WaitingQueue_head=temp->next;
		temp->next=NULL;
	}
	else //remove in the middle
	{
		temp1->next=temp->next;
		temp->next=NULL;
	}
	return temp->data;
}

/*
	RemoveFromWaitingQueueWithPID
	developed version
	not in use finally
*/
PCB *RemoveFromWaitingQueueWithPID(int pid)
{
	if (WaitingQueue_head==NULL)
	{
		return NULL;
	}

	temp=WaitingQueue_head;
	temp1=temp;
	while (temp->data->process_id!=pid)
	{
		temp1=temp;
		temp=temp->next;
		if (temp==NULL)
			return NULL;
	}
	//remove the first
	if (temp1==temp)
	{
		WaitingQueue_head=temp->next;
		temp->next=NULL;
	}
	else //remove in the middle
	{
		temp1->next=temp->next;
		temp->next=NULL;
	}
	return temp->data;
}

/************************************************************
						SET_PRINTER
To set the scheduler printer as JB wanted.

In: void
Out: void
*************************************************************/
void set_printer()
{
	TQ temp_print;
	if (ReadyQueue_head!=NULL)
	{
		temp_print=ReadyQueue_head;
		CALL( SP_setup( SP_READY_MODE, temp_print->data->process_id ) );
		while (temp_print->next!=NULL)
		{
			temp_print=temp_print->next;
			CALL( SP_setup( SP_READY_MODE, temp_print->data->process_id ) );
		} 
	}
	if (TimerQueue_head!=NULL)
	{
		temp_print=TimerQueue_head;
		CALL( SP_setup( SP_WAITING_MODE, temp_print->data->process_id ) );
		while (temp_print->next!=NULL)
		{
			temp_print=temp_print->next;
			CALL( SP_setup( SP_WAITING_MODE, temp_print->data->process_id ) );
		} 
	}
	if (WaitingQueue_head!=NULL)
	{
		temp_print=WaitingQueue_head;
		CALL( SP_setup( SP_SUSPENDED_MODE, temp_print->data->process_id ) );
		while (temp_print->next!=NULL)
		{
			temp_print=temp_print->next;
			CALL( SP_setup( SP_SUSPENDED_MODE, temp_print->data->process_id ) );
		} 
	}
}

/*
	developed version
	not in use finally
*/
BOOL IsSafeNow(short disk_id)
{
	TQ temp;
	PrintReadyQueue();
	PrintWaitingQueue();

	if (current_pcb->disk_id_index==disk_id)
		return FALSE;

	if (ReadyQueue_head==NULL)
		return TRUE;
	
	temp=ReadyQueue_head;
	if (temp->data->disk_id_index==disk_id)
		return FALSE;
	while (temp->next!=NULL)
	{
		temp=temp->next;
		if (temp->data->disk_id_index==disk_id)
		return FALSE;
	}

	return TRUE;
}

/************************************************************************
					INTERRUPT_HANDLER
When the Z502 gets a hardware interrupt, it transfers control to
this routine in the OS. 

First extract the first in the TimerQueue, and decided whether to add it to ReadyQueue
or WaitingQueue. Check if the others in TimerQueue also need to be handled this way.
If still some process in TimerQueue, reset the Timer.

In: void
Out: void

What in phase 2:
used to help scheduling, put process from waitingqueue back to readyqueue

How in phase 2:
　　As each time the disk successfully writes or reads, a disk interrupt would occurred
  and we'll go into the interrupt handler. In the interrupt hander, we fetch the related 
  process, and check if the process has failed a write/read before. If not, we directly 
  put the process to Ready Queue. Otherwise, we need to write/read again, then we add the
  process back to the head of the Waiting Queue. The next time an disk interrupt occurred
  because of this operation, we will then put the process to the Ready Queue.
************************************************************************/
void interrupt_handler( void ) {
    INT32              device_id;
    INT32              status;
    INT32              Index = 0;
	INT32			   Timer_Facebook;
	INT32			   Timer_Twitter;
	long				sleep_time;
	PCB	                *pcb_1;
    static BOOL        remove_this_in_your_code = TRUE;   /** TEMP **/
    static INT32       how_many_interrupt_entries = 0;    /** TEMP **/
	static int         testing = 0;

    // Get cause of interrupt
    MEM_READ(Z502InterruptDevice, &device_id ); 
	//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
	
	while (device_id !=-1 )
	{	
		// Set this device as target of our query
		MEM_WRITE(Z502InterruptDevice, &device_id );
		// Now read the status of this device
		MEM_READ(Z502InterruptStatus, &status );

		if (status==ERR_BAD_PARAM || status==ERR_NO_PREVIOUS_WRITE ||status==ERR_DISK_IN_USE)
		{
			printf("@@@@@@ ERROR %d detected on device: %d!@@@@@@@@\n",status,device_id);
		}

		if (device_id==4)
		{
			//only interrupt_handler thread could remove thing from the timer queue
			//we must keep the routine as a whole
			// remove 1st item from TimerQueue
			pcb_1 = (PCB *)malloc(sizeof(PCB));

			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
			pcb_1 = RemoveFromTimerQueue(); //extract the first in the TimerQueue

			//if not to be suspended
			if (suspend_flag[pcb_1->process_id]==-1)
			{
				AddToReadyQueue(pcb_1);
				if (!FULL_PRINT) //print format as jb asked
				{
				CALL( SP_setup_action( SP_ACTION_MODE, "Ready" ) );
				CALL( SP_setup( SP_TARGET_MODE, (INT32)pcb_1->process_id ) );
				CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
				set_printer();
				if ((count++)%15==0) 
				{
					CALL( SP_print_header() );
				} 
				CALL( SP_print_line() );
				}
			}
			else
			{
				AddToWaitingQueue(pcb_1); //it was suppoed to suspended when in TimerQueue
				if (!FULL_PRINT) //print format as jb asked
				{
				CALL( SP_setup_action( SP_ACTION_MODE, "Suspend" ) );
				CALL( SP_setup( SP_TARGET_MODE, (INT32)pcb_1->process_id ) );
				CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
				set_printer();
				if ((count++)%15==0)
				{
					CALL( SP_print_header() );
				} 
				CALL( SP_print_line() );
				}
			}

			//check and move all the items that has the same wake_up_time and make decision repeadly
			while (TimerQueue_head!=NULL && TimerQueue_head->data->wake_up_time==pcb_1->wake_up_time)
			{
				pcb_1 = RemoveFromTimerQueue();
				if (suspend_flag[pcb_1->process_id]==-1)
				{
					AddToReadyQueue(pcb_1);
					if (!FULL_PRINT)
					{
					CALL( SP_setup_action( SP_ACTION_MODE, "Ready" ) );
					CALL( SP_setup( SP_TARGET_MODE, (INT32)pcb_1->process_id ) );
					CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
					set_printer();
					if ((count++)%15==0)
					{
						CALL( SP_print_header() );
					} 
					CALL( SP_print_line() );
					}
				}
				else
				{
					AddToWaitingQueue(pcb_1);
					if (!FULL_PRINT)
					{
					CALL( SP_setup_action( SP_ACTION_MODE, "Suspend" ) );
					CALL( SP_setup( SP_TARGET_MODE, (INT32)pcb_1->process_id ) );
					CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
					set_printer();
					if ((count++)%15==0)
					{
						CALL( SP_print_header() );
					} 
					CALL( SP_print_line() );
					}
				}
			}

			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);

			//reset the timer
			if (TimerQueue_head!=NULL)
			{
				ZCALL( MEM_READ(Z502ClockStatus, &Timer_Facebook));//read current time
				sleep_time=TimerQueue_head->data->wake_up_time-Timer_Facebook;
				Timer_Twitter=(INT32)sleep_time; //compute relevant time to sleep
				MEM_WRITE( Z502TimerStart, &Timer_Twitter );
			}

			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
	
		}
		else if( device_id==5 || device_id==6 || device_id==7)
		{
			PCB *temp;
			short disk_ID;

			switch (device_id)
			{
			case 5:
				disk_ID=1;
				break;
			case 6:
				disk_ID=2;
				break;
			case 7:
				disk_ID=3;
				break;
			default:
				printf("@@@@@@ Error Here! @@@@@@\n");
				break;
			}
			
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			
			if (just_for_testing)
			{
				printf("disk interrupt:%d\n",disk_ID);
			}
			

			temp=RemoveFromWaitingQueueWithDiskID(disk_ID);

			if(temp!=NULL)
			{
				if (temp->step_index==GO_TO_WRITE1)
				{
					INT32 Temp;
					MEM_WRITE( Z502DiskSetID, (INT32 *)&temp->Z502_ARG1.VAL );
					MEM_READ( Z502DiskStatus, &Temp );
					if (Temp == DEVICE_FREE)
					{
						MEM_WRITE( Z502DiskSetSector, (INT32 *)&temp->Z502_ARG2.VAL);
						MEM_WRITE( Z502DiskSetBuffer, (INT32 *)temp->Z502_ARG3.PTR );
						Temp = 1;                        // Specify a write
						MEM_WRITE( Z502DiskSetAction, &Temp );
						Temp = 0;                        // Must be set to 0
						MEM_WRITE( Z502DiskStart, &Temp );

						temp->step_index=GO_TO_WRITE2;
						if (flag==19)
						{
							//*temp->is_dirty_pointer=FALSE;
						}

						AddBackToWaitingQueue(temp);
						
						if (just_for_testing)
						{
							printf("in use!\n");
							PrintWaitingQueue();
						}
					}
					else
					{
						//if in use, then add back
						AddBackToWaitingQueue(temp);
						printf("%%%%%%%%%%%Something wrong here!%%%%%%%%%");
					}
				}
				else if(temp->step_index==GO_TO_READ1)
				{
					INT32 Temp;

					MEM_WRITE( Z502DiskSetID, (INT32 *)&temp->Z502_ARG1.VAL );
					MEM_READ( Z502DiskStatus, &Temp );
					if (Temp == DEVICE_FREE)
					{
						MEM_WRITE( Z502DiskSetSector, (INT32 *)&temp->Z502_ARG2.VAL);
						MEM_WRITE( Z502DiskSetBuffer, (INT32 *)temp->Z502_ARG3.PTR );
						Temp = 0;                        // Specify a write
						MEM_WRITE( Z502DiskSetAction, &Temp );
						Temp = 0;                        // Must be set to 0
						MEM_WRITE( Z502DiskStart, &Temp );

						temp->step_index=GO_TO_READ2;

						AddBackToWaitingQueue(temp);

						if (just_for_testing)
						{
							printf("Read:%d-%d-%d-%d\n", ((CONTAINER *)temp->Z502_ARG3.PTR)->int_data[0],((CONTAINER *)temp->Z502_ARG3.PTR)->int_data[1],((CONTAINER *)temp->Z502_ARG3.PTR)->int_data[2],((CONTAINER *)temp->Z502_ARG3.PTR)->int_data[3]);
							printf("in use2!\n");
							PrintWaitingQueue();
						}
					}
					else
					{
						//if in use, then add back
						AddBackToWaitingQueue(temp);
						printf("%%%%%%%%%%%Something wrong here!%%%%%%%%%");
					}
				}
				else
				{
					//temp->step_index==GO_TO_WRITE2 or temp->step_index==-1 or temp->step_index==GO_TO_READ2
					AddToReadyQueue(temp);
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
					suspend_flag[temp->process_id] = -1;
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
					
					if (just_for_testing)
						{
							printf("PID %d transformed from WaitingQueue to ReadyQueue!\n",temp->process_id);
							PrintReadyQueue();
						}
				}
			}	
			
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
		}
		else
		{
			if (just_for_testing)
			{
				printf("Interrupt %d not be handled!\n",device_id);
			}
		}
				

		Index = 0;
		MEM_WRITE(Z502InterruptClear, &Index );	

		MEM_READ(Z502InterruptDevice, &device_id );

		if(just_for_testing!=0&&device_id!=-1)
		{
			printf("Multiple interrupts: %d\n",device_id);
		}

	}
}                                       /* End of interrupt_handler */

/************************************************************************
    FAULT_HANDLER
        The beginning of the OS502.  Used to receive hardware faults.

	What in phase 2:
	to handle page fault

	How in phase 2:
	implement LRU algorithm, page fetch algorithm, page replacement algorithm
	detailed discription please refers to the documentation

************************************************************************/
void fault_handler( void )
    {
    INT32       device_id;
    INT32       status;
    INT32       Index = 0;
	static INT16  first = TRUE;
	INT16		base=(INT16) Z502_ARG1.VAL/PGSIZE;
	INT16		offset=(INT16) Z502_ARG1.VAL%PGSIZE;
	short       free_frame;
	BOOL        inner_flag=TRUE;
	int			disk_id=current_pcb->process_id+1;
	int         process_id,j;
	short       *step_index;
	INT32		a,b,c;

    // Get cause of interrupt
    MEM_READ(Z502InterruptDevice, &device_id ); 
    // Set this device as target of our query
    MEM_WRITE(Z502InterruptDevice, &device_id );
    // Now read the status of this device
    MEM_READ(Z502InterruptStatus, &status );

    printf( "Fault_handler: Found vector type %d with value %d\n", 
                        device_id, status );

	if (Z502_PAGE_TBL_ADDR==NULL)
	{
		Z502_PAGE_TBL_LENGTH = 1024;
		Z502_PAGE_TBL_ADDR 
			= (UINT16 *)calloc( sizeof(UINT16), Z502_PAGE_TBL_LENGTH );
		address_storgae[disk_id-1]=Z502_PAGE_TBL_ADDR;
	}
		

	if (Z502_ARG1.VAL > VIRTUAL_MEM_PGS*PGSIZE-4)
	{
		printf("Invalid page table size request!\n");
		Z502_HALT();
	}

	//This is very special because PTBL_VALID_BIT == 0x8000;
	//if invalid and not in-disk
	if(Z502_PAGE_TBL_ADDR[base]>>15==0)
	{
		if ((Z502_PAGE_TBL_ADDR[base]&0x1000)>>12==0)
		{
			//make sure only have ceraain number of frames
			free_frame=RemoveFromFreeFrameList();
			if (free_frame!=-1)
			{
				Z502_PAGE_TBL_ADDR[base] =free_frame | PTBL_VALID_BIT;
				++memory_organizer_index;
				memory_organizer[memory_organizer_index]=&Z502_PAGE_TBL_ADDR[base];
				process_storage[memory_organizer_index]=disk_id-1;
			}
			else
			{
				while(1)
				{
					reference_index2=(++reference_index2)%PHYS_MEM_PGS;
					
					if (((*memory_organizer[reference_index2])&PTBL_REFERENCED_BIT)>>13==1)
					{
						//将此位置零 effecitve
						*memory_organizer[reference_index2]&=(~PTBL_REFERENCED_BIT);
					}
					else
					{
						short frame_number;
						int Temp;
						int disk_id2;

						frame_number=(short) (*memory_organizer[reference_index2])&0x0fff;									
						
						inner_flag=FALSE;
						MEM_WRITE(Z502InterruptClear, &Index );

						current_pcb->is_dirty_pointer=&is_dirty[reference_index2];
						step_index=&current_pcb->step_index;
						process_id=process_storage[reference_index2];
						
						if (flag==19)
						{
							disk_id2=process_id+1;
							MEM_WRITE( Z502DiskSetID, &disk_id2 );
							MEM_READ( Z502DiskStatus, &Temp );
							if ( Temp != DEVICE_FREE ) 
							{
								Z502_PROGRAM_COUNTER--;
								AddToReadyQueue(current_pcb);
								dispatcher();
								return;
							}
						}

						os_disk_write(process_id+1, memory_organizer[reference_index2]-address_storgae[process_id], (char *) &MEMORY[frame_number*PGSIZE]);

						if (flag=19)
						{
							printf("%d\n",*step_index);

							if (*step_index==GO_TO_WRITE1)
								is_dirty[reference_index2]=TRUE;
						}

						printf("Swipe out:%d to disk %d sector %d from frame %d\n", ((CONTAINER *)&MEMORY[frame_number*PGSIZE])->int_data[0],process_id+1, memory_organizer[reference_index2]-address_storgae[process_id],frame_number);

						(*memory_organizer[reference_index2])|=0x1000;//in disk
						(*memory_organizer[reference_index2])&=~PTBL_VALID_BIT;//invalid

						Z502_PAGE_TBL_ADDR[base] = frame_number | PTBL_VALID_BIT;
						memory_organizer[reference_index2]=&Z502_PAGE_TBL_ADDR[base];
						process_storage[reference_index2]=disk_id-1;
						//printf("end\n");
						break;
					}
				}
			}
		}
		else
		{
			free_frame=RemoveFromFreeFrameList();
			if (free_frame!=-1)
			{
				int Temp;
				//step_back=TRUE;
				os_disk_read(disk_id, base, (char *) &MEMORY[free_frame*PGSIZE]);

				Z502_PAGE_TBL_ADDR[base] =free_frame | PTBL_VALID_BIT;
				++memory_organizer_index;
				memory_organizer[memory_organizer_index]=&Z502_PAGE_TBL_ADDR[base];
				process_storage[memory_organizer_index]=disk_id-1;

			}
			else
			{
				while(1)
				{
					//reference_index=(++reference_index)%VIRTUAL_MEM_PGS;
					reference_index2=(++reference_index2)%PHYS_MEM_PGS;
					//if (Z502_PAGE_TBL_ADDR[reference_index]>>15==1)
					{
						if (((*memory_organizer[reference_index2])&PTBL_REFERENCED_BIT)>>13==1)
						{
							//将此位置零
							(*memory_organizer[reference_index2])&=~PTBL_REFERENCED_BIT;
						}
						else
						{
							//选择这个reference为victim
							short frame_number;
							int Temp;
							long test_buffer_b;
							long       container_b;
							CONTAINER *container_a;
							container_b = ( long )calloc( 1, sizeof( CONTAINER ) );
							test_buffer_b = ( long )calloc( 1, sizeof( CONTAINER ) );
							container_a = (CONTAINER *) container_b;

							frame_number=(short) (*memory_organizer[reference_index2])&0x0fff;

							inner_flag=FALSE;
							MEM_WRITE(Z502InterruptClear, &Index );

							process_id=process_storage[reference_index2];
							os_disk_write(process_id+1, memory_organizer[reference_index2]-address_storgae[process_id], (char *) &MEMORY[frame_number*PGSIZE]);

							printf("Swipe out:%d to disk %d sector %d from frame %d\n", ((CONTAINER *)&MEMORY[frame_number*PGSIZE])->int_data[0],disk_id,memory_organizer[reference_index2]-address_storgae[disk_id-1],frame_number);
						
							(*memory_organizer[reference_index2])|=0x1000;//in disk
							(*memory_organizer[reference_index2])&=~PTBL_VALID_BIT;//invalid
								
							//将东西swipe in
							//step_back=TRUE;
								
							os_disk_read(disk_id, base, (char *) &MEMORY[frame_number*PGSIZE]);
							printf("Swipe in:%d from disk %d sector %d to fram %d\n", ((CONTAINER *)&MEMORY[frame_number*PGSIZE])->int_data[0],disk_id,base,frame_number);

							Z502_PAGE_TBL_ADDR[base] =frame_number | PTBL_VALID_BIT;
							memory_organizer[reference_index2]=&Z502_PAGE_TBL_ADDR[base];
							process_storage[reference_index2]=disk_id-1;
							break;
						}
					}
				}
			}
		}
	}
		
	//printf("(UINT16)Z502_ARG1.VAL%PGSIZE>12: %d\n",(UINT16)Z502_ARG1.VAL%PGSIZE);
	//next page also invalid and not in-disk
	if((offset > PGSIZE - 4) && (Z502_PAGE_TBL_ADDR[base+1]>>15==0))
	{
		//to be updated
		if ((Z502_PAGE_TBL_ADDR[base+1]&0x1000)>>12==0)
		{
			free_frame=RemoveFromFreeFrameList();
			if (free_frame!=-1)
			{
				Z502_PAGE_TBL_ADDR[base+1] =free_frame | PTBL_VALID_BIT;
				++memory_organizer_index;
				memory_organizer[memory_organizer_index]=&Z502_PAGE_TBL_ADDR[base+1];
				process_storage[memory_organizer_index]=disk_id-1;
			}
			else
			{	
				while(1)
				{
					reference_index2=(++reference_index2)%PHYS_MEM_PGS;

					if (((*memory_organizer[reference_index2])&PTBL_REFERENCED_BIT)>>13==1)
					{
						//将此位置零 effecitve
						*memory_organizer[reference_index2]&=(~PTBL_REFERENCED_BIT);
					}
					else
					{
						//选择这个reference为victim
						short frame_number;
						int Temp;
						frame_number=(short) (*memory_organizer[reference_index2])&0x0fff;
						
						inner_flag=FALSE;
						MEM_WRITE(Z502InterruptClear, &Index );

						process_id=process_storage[reference_index2];
						os_disk_write(process_id+1, memory_organizer[reference_index2]-address_storgae[process_id], (char *) &MEMORY[frame_number*PGSIZE]);
								
						(*memory_organizer[reference_index2])|=0x1000;//in disk
						(*memory_organizer[reference_index2])&=~PTBL_VALID_BIT;//invalid

						Z502_PAGE_TBL_ADDR[base+1] =frame_number | PTBL_VALID_BIT;
						memory_organizer[reference_index2]=&Z502_PAGE_TBL_ADDR[base+1];
						process_storage[reference_index2]=disk_id-1;
						break;
					}
				}
			}
		}
		else
		{
			//here the information is in disk
			free_frame=RemoveFromFreeFrameList();
			if (free_frame!=-1)
			{
				//step_back=TRUE;

				inner_flag=FALSE;
				MEM_WRITE(Z502InterruptClear, &Index );

				os_disk_read(disk_id, base+1, (char *) &MEMORY[free_frame*PGSIZE]);
				Z502_PAGE_TBL_ADDR[base+1] =free_frame | PTBL_VALID_BIT;
				++memory_organizer_index;
				memory_organizer[memory_organizer_index]=&Z502_PAGE_TBL_ADDR[base+1];
				process_storage[memory_organizer_index]=disk_id-1;
			}
			else
			{					
				while(1)
				{
					//reference_index=(++reference_index)%VIRTUAL_MEM_PGS;
					reference_index2=(++reference_index2)%PHYS_MEM_PGS;
					//if (Z502_PAGE_TBL_ADDR[reference_index]>>15==1)
					{
						if (((*memory_organizer[reference_index2])&PTBL_REFERENCED_BIT)>>13==1)
						{
							//将此位置零
							(*memory_organizer[reference_index2])&=~PTBL_REFERENCED_BIT;
						}
						else
						{
							//选择这个reference为victim
							short frame_number;
							int Temp;
							long test_buffer_b;
							long       container_b;
							CONTAINER *container_a;
							container_b = ( long )calloc( 1, sizeof( CONTAINER ) );
							test_buffer_b = ( long )calloc( 1, sizeof( CONTAINER ) );
							container_a = (CONTAINER *) container_b;

							frame_number=(short) (*memory_organizer[reference_index2])&0x0fff;

							inner_flag=FALSE;
							MEM_WRITE(Z502InterruptClear, &Index );

							process_id=process_storage[reference_index2];
							os_disk_write(process_id+1, memory_organizer[reference_index2]-address_storgae[process_id], (char *) &MEMORY[frame_number*PGSIZE]);
								
							(*memory_organizer[reference_index2])|=0x1000;//in disk
							(*memory_organizer[reference_index2])&=~PTBL_VALID_BIT;//invalid
							
							os_disk_read(disk_id, base+1, (char *) &MEMORY[frame_number*PGSIZE]);

							Z502_PAGE_TBL_ADDR[base+1] =frame_number | PTBL_VALID_BIT;
							memory_organizer[reference_index2]=&Z502_PAGE_TBL_ADDR[base+1];
							process_storage[reference_index2]=disk_id-1;
							break;
						}
					}
				}
			}
		}
	}

	if (flag==11)
	{
		printf("**********test1k: User cannot read memory directly!************\n");
		Z502_HALT();
	}
	
	//MP_setup( (INT32)(free_frame), (INT32) disk_id-1, (INT32) base+1, (INT32) 4 );		
	for ( c = 0; c < 64; c++)
    {
		if (memory_organizer[c]!=NULL)
		{
			MP_setup( (INT32)((*memory_organizer[c])&0x0fff), (INT32)process_storage[c], (INT32)(memory_organizer[c]-address_storgae[process_storage[c]]), (INT32)((*memory_organizer[c])&0xe000)>>13 );
		}
			
	}
	MP_print_line();

    // Clear out this device - we're done with it
	if (inner_flag)
	{
		MEM_WRITE(Z502InterruptClear, &Index );
	}
}                                       /* End of fault_handler */

/************************************************************************
						 AddToReadyQueue
Add a ceratin process to ReadyQueue

In: a PCB typed process
OUt: void
************************************************************************/
void AddToReadyQueue(PCB *pcb)
	{
		//we assume that a pcb with a smaller priority has an advantage over the ones with greater priorities, as test1h defined
		TQ temp_ready1;
		temp_ready1=(struct Node *)malloc(sizeof(struct Node ));
		//if empty, just add to be the first
		if (ReadyQueue_head==NULL) //ReadyQueue=NULL is a huge mistake
		{
			temp_ready1->data = pcb;
			temp_ready1->next = NULL;
			ReadyQueue_head  = temp_ready1;
		}
		
		//if ReadyQueue is not empty, compare the priority and insert to a proper position
		else 
		{			
			TQ temp_ready2,temp_ready3;
			
			if (ReadyQueue_head->data->priority>pcb->priority)
			{
				temp_ready1->data=pcb;
				temp_ready1->next=ReadyQueue_head;
				ReadyQueue_head=temp_ready1;
			}
			else 
			{
				temp_ready1=ReadyQueue_head;
				temp_ready2=temp_ready1;
				while((temp_ready1->next!=NULL)&&(temp_ready1->data->priority<=pcb->priority))
				{
					temp_ready2=temp_ready1;
					temp_ready1=temp_ready1->next;
				}
				//situation 1: need to add to the end
				temp_ready3=(struct Node *)malloc(sizeof(struct Node ));
				temp_ready3->data=pcb;
				if (temp_ready1->data->priority<=pcb->priority)
				{
					temp_ready3->next=NULL;
					temp_ready1->next=temp_ready3;
				}
				//situation 2: need to add to between temp_ready2 and temp_ready1
				else
				{
					temp_ready3->next=temp_ready1;
					temp_ready2->next=temp_ready3;
				}
			}
		}
}

/************************************************************************
							 AddToTimerQueue
Add a ceratin process to TimerQueue

In: a PCB typed process
OUt: void
************************************************************************/
void AddToTimerQueue(PCB *pcb)
	{
		//if TimerQueue is empty
		TQ temp_timer1;
		temp_timer1=(struct Node *)malloc(sizeof(struct Node ));
		if (TimerQueue_head==NULL)
		{
			temp_timer1->data = pcb;
			temp_timer1->next = NULL;
			TimerQueue_head  = temp_timer1;
		}
		//if TimerQueue is not empty, compare the wake_up_time and insert to a proper position
		//they are listed in a increased way in terms of wake_up_time
		else 
		{			
			TQ temp_timer2,temp_timer3;
			
			if (TimerQueue_head->data->wake_up_time>pcb->wake_up_time)
			{
				temp_timer1->data=pcb;
				temp_timer1->next=TimerQueue_head;
				TimerQueue_head=temp_timer1;
			}
			else 
			{
				temp_timer1=TimerQueue_head;
				temp_timer2=temp_timer1;
				while((temp_timer1->next!=NULL)&&(temp_timer1->data->wake_up_time<=pcb->wake_up_time))
				{
					temp_timer2=temp_timer1;
					temp_timer1=temp_timer1->next;
				}
				//situation 1: need to add to the end
				temp_timer3=(struct Node *)malloc(sizeof(struct Node ));
				temp_timer3->data=pcb;
				if (temp_timer1->data->wake_up_time<=pcb->wake_up_time)
				{
					temp_timer3->next=NULL;
					temp_timer1->next=temp_timer3;
				}
				//situation 2: need to add to between temp_timer2 and temp_timer1
				else
				{
					temp_timer3->next=temp_timer1;
					temp_timer2->next=temp_timer3;
				}
			}
		}
}

/*
	AddToDiskQueue
	developed version
	actually not in use
*/
void AddToDiskQueue(INT16 disk_id, int pid)
{
	DQ temp_disk_queue_element,temp1;
	DiskQ *temp;
	temp=(struct DKQ *)malloc(sizeof(struct DKQ));
	temp->disk_id=disk_id;
	temp->pid=pid;
	temp_disk_queue_element=(struct NodeD *)malloc(sizeof(struct NodeD ));
	temp_disk_queue_element->data=temp;
	temp_disk_queue_element->next = NULL;

	//if messageQueue is empty, add to be the first one
	if (DiskQueue_head==NULL)
	{	
		DiskQueue_head  = temp_disk_queue_element;
	}
	else // add to the end
	{			
		temp1=DiskQueue_head;
		while(temp1->next !=NULL)
			temp1=temp1->next ;
		temp1->next = temp_disk_queue_element;
	}
}

/*
	AddToFreeFrameList
	What: this is used to construct a free frame list in os_init
	later we could directly check the free frame list to see
	if the system contains any free frames any more

	How:
	just add new node to a linked list
*/
void AddToFreeFrameList(short frame_number)
{
	FF temp;
	temp=(struct NodeF *)malloc(sizeof(struct NodeF));
	temp->frame_number=frame_number;

	//add the new node to the head of the FreeFrameList
	temp->next=FreeFrameList_head;
	FreeFrameList_head=temp;
}

/************************************************************************
						 AddToMessageQueue
Add a message to MessageQueue

In: a message
Out: void
************************************************************************/
void AddToMessageQueue(Message *msg)
{
	MQ msg1;
	msg1=(struct NodeM *)malloc(sizeof(struct NodeM ));
	msg1->data = msg;
	msg1->next = NULL;

	//if messageQueue is empty, add to be the first one
	if (MessageQueue_head==NULL)
	{	
		MessageQueue_head  = msg1;
	}
	else // add to the end
	{			
		MQ temp1;
		temp1=MessageQueue_head;
		while(temp1->next !=NULL)
			temp1=temp1->next ;
		temp1->next = msg1;
	}
}

/************************************************************************
						 AddToWaitingQueue
Add a ceratin process to WaitingQueue

In: a PCB typed process
OUt: void
************************************************************************/
void AddToWaitingQueue(PCB *pcb_waiting)
{
	TQ temp_Waiting1;
	temp_Waiting1=(struct Node *)malloc(sizeof(struct Node ));
	temp_Waiting1->data = pcb_waiting;
	temp_Waiting1->next = NULL;

	//if empty, just add as the first one
	if (WaitingQueue_head==NULL)
	{	
		WaitingQueue_head  = temp_Waiting1;
	}
	//if WaitingQueue is not empty, add to the end
	else 
	{			
		TQ temp1;
		temp1=WaitingQueue_head ;
		while(temp1->next !=NULL)
			temp1=temp1->next ;
		temp1->next = temp_Waiting1;
	}
}

/*
	AddBackToWaitingQueue
	
	What: used in interrupt_handler
	when a process writes/reads again, it was put back to the head of the 
	WaitingQueue, waiting for the next interrupt

	How: add a new node to the head of a linked list
*/
void AddBackToWaitingQueue(PCB *pcb_waiting)
{
	//add to the first of the queue
	TQ temp_Waiting1;
	temp_Waiting1=(struct Node *)malloc(sizeof(struct Node ));
	temp_Waiting1->data = pcb_waiting;

	temp_Waiting1->next = WaitingQueue_head;
	WaitingQueue_head = temp_Waiting1;
}

/************************************************************************
						 StartTimer
Set the Timer.

In: time to sleep
Out: void
************************************************************************/
void StartTimer(long x)
	{
	INT32		        Status;
	INT32				Temp;
	INT32				Timer_Google;

	if (x==0 ) //if sleeping time is 0, just add the running process to ReadyQueue
	{
		//printf("Stop A\n");
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		AddToReadyQueue(current_pcb);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
	}
	else //otherwise find if timer is working
	{
		ZCALL( MEM_READ(Z502ClockStatus, &Timer_Google));
		current_pcb->wake_up_time=Timer_Google+x;
		Temp = x;  

		MEM_READ( Z502TimerStatus, &Status );

		//printf("Stop B\n");
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		if (Status == DEVICE_FREE )//if timer is not working, just set the timer
		{
			MEM_WRITE( Z502TimerStart, &Temp );
			//If you start the timer, and it’s already timing something, 
			//it simply overwrites what was previously happening and performs your new operation.
		}
		else //if timer is working but current sleep time is less than the one that has setted the timer 
		{            //or the TimerQueue is empty, reset or set the timer
			if (TimerQueue_head==NULL || TimerQueue_head->data->wake_up_time>current_pcb->wake_up_time)
			{
				MEM_WRITE( Z502TimerStart, &Temp );
			}
		}
		
		AddToTimerQueue(current_pcb);

		if (!FULL_PRINT) //print format as jb asked
		{
		CALL( SP_setup_action( SP_ACTION_MODE, "Wait" ) );
		CALL( SP_setup( SP_TARGET_MODE, (INT32)current_pcb->process_id ) );
		CALL( SP_setup( SP_RUNNING_MODE, (INT32)current_pcb->process_id ) );
		set_printer();
		if ((count++)%15==0)
		{
			CALL( SP_print_header() );
		} 
		CALL( SP_print_line() );
		}

		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
	}
	
}

/**********************************************************************************
								OS_SEND_MESSAGE
To send a message.

In: target process id, content of the message, length of the message
**********************************************************************************/
void os_send_message(long target_id, void *message, long send_length)
{
	size_t size;
	Message *msg1;
	msg1=(struct MG *)malloc(sizeof( struct MG ));
	*(long *) Z502_ARG4.PTR = ERR_SUCCESS ;

	if (target_id<-1||target_id>=MAX_PROCESS_NUMBER) //whether voilate the process id
	{
		*(long *) Z502_ARG4.PTR = ERR_NONE_PROCESS_ID ;
	}
	else if (target_id!=-1&&process_pid[(int)target_id]==-1) //whether exists such a process
	{
		*(long *) Z502_ARG4.PTR = ERR_NONE_PROCESS_ID ;
	}
	else if (send_length<=0||send_length>MAX_LEGAL_MESSAGE_LENGTH) //whether voilate the lengthe of legal message
	{
		*(long *) Z502_ARG4.PTR = ERR_ILLEGAL_MESSAGE_LENGTH;
	}
	else
	{
		TQ temp1,temp2;
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);

		temp1=WaitingQueue_head;
		
		//if some process in WaitingQueue because of not receiving such a message, then invoke the process
		//extracting the process out of WaitingQueue and putting it to the ReadyQueue
		if (target_id==-1) //if send a broadcast
		{
			if (temp1!=NULL)
			{
				if (temp1->data->message_flag==TRUE) //find anyone who wants a message
				{
					WaitingQueue_head=WaitingQueue_head->next;
					//temp1->data->message_flag=FALSE;
					AddToReadyQueue(temp1->data);
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
					suspend_flag[temp1->data->process_id]=-1;
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
					free(temp1);
				}
				else
				{
					while(temp1->next!=NULL)
					{
						temp2=temp1;
						temp1=temp1->next;
						if (temp1->data->message_flag==TRUE) //find anyone who wants a message
						{
							temp2->next=temp1->next;
							//temp1->data->message_flag=FALSE;
							AddToReadyQueue(temp1->data);
							Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
							suspend_flag[temp1->data->process_id]=-1;
							Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
							free(temp1);
							break;
						}
					}
				}
			} 
		}
		else //if not a broadcast
		{
			if (temp1!=NULL)
			{
				//find the target process and make sure it is in need of messages
				if (target_id==temp1->data->process_id&&temp1->data->message_flag==TRUE) 
				{
					WaitingQueue_head=WaitingQueue_head->next;
					//temp1->data->message_flag=FALSE;
					AddToReadyQueue(temp1->data);
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
					suspend_flag[temp1->data->process_id]=-1;
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
					free(temp1);
				}
				else
				{
					while(temp1->next!=NULL)
					{
						temp2=temp1;
						temp1=temp1->next;
						if (target_id==temp1->data->process_id&&temp1->data->message_flag==TRUE)
						{
							temp2->next=temp1->next;
							//temp1->data->message_flag=FALSE;
							AddToReadyQueue(temp1->data);
							Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
							suspend_flag[temp1->data->process_id]=-1;
							Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
							free(temp1);
							break;
						}
					}
				}
			}
		}
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);

		//get the basic information and store the message in messageQueue
		msg1->source_pid=current_pcb->process_id;
		msg1->target_pid=target_id;
		msg1->send_length=send_length;
		size=strlen((char *)message);
		if (size>(unsigned)send_length)
		{
			strncpy(msg1->message_buffer,(const char*)message,send_length);
		}
		else
		{
			strcpy(msg1->message_buffer,(const char*)message);
		}
			
		if (++message_count>MAX_MESSAGE_NUMBER)
		{
			*(long *) Z502_ARG4.PTR = ERR_EXCEED_MAX_MESSAGE_NUMBER ;
			return;
		}
		else
		{
			AddToMessageQueue(msg1);
		}
	}
}

/**********************************************************************************
								OS_RECEIVE_MESSAGE
To receive a message.

In: source process id, buffer for a message, length for the message
**********************************************************************************/
void os_receive_message(long source_id, void *msg_buffer, long receive_length)
{
	
	*(long *) Z502_ARG4.PTR = -1 ;
	*(long *) Z502_ARG5.PTR = -2 ;
	*(long *) Z502_ARG6.PTR = ERR_SUCCESS ;

	if (source_id<-1||source_id>=MAX_PROCESS_NUMBER) //whether voilate the process id
	{
		*(long *) Z502_ARG6.PTR = ERR_NONE_PROCESS_ID ;
	}
	else if (source_id!=-1&&process_pid[(int)source_id]==-1) //whether exists such a process
	{
		*(long *) Z502_ARG6.PTR = ERR_NONE_PROCESS_ID ;
	}
	else if (receive_length<=0||receive_length>MAX_LEGAL_MESSAGE_LENGTH) //whether voilate length request
	{
		*(long *) Z502_ARG6.PTR = ERR_ILLEGAL_MESSAGE_LENGTH;
	}
	else
	{
		MQ temp1,temp2;
		if(MessageQueue_head!=NULL)
		{
			temp1=MessageQueue_head;
			if (source_id==-1)
			{
				//a broadcast or a message target at the running process
				if (temp1->data->target_pid==-1||temp1->data->target_pid==current_pcb->process_id)
				{
					if (temp1->data->send_length>receive_length)
					{
						*(long *) Z502_ARG6.PTR = ERR_ILLEGAL_MESSAGE_LENGTH;
						return;
					}
					else
					{
						*(long *) Z502_ARG4.PTR = temp1->data->send_length;
						*(long *) Z502_ARG5.PTR = temp1->data->source_pid;
						strcpy((char *)msg_buffer,temp1->data->message_buffer);
						current_pcb->message_flag=FALSE;
						MessageQueue_head=MessageQueue_head->next;
						message_count--;
						return;
					}
				}
				else
				{
					while(temp1->next!=NULL)
					{
						temp2=temp1;
						temp1=temp1->next;
						//a broadcast or a message target at the running process----> continued search the messageQueue
						if (temp1->data->target_pid==-1||temp1->data->target_pid==current_pcb->process_id)
						{
							if (temp1->data->send_length>receive_length)
							{
								*(long *) Z502_ARG6.PTR = ERR_ILLEGAL_MESSAGE_LENGTH;
								return;
							}
							else
							{
								*(long *) Z502_ARG4.PTR = temp1->data->send_length;
								*(long *) Z502_ARG5.PTR = temp1->data->source_pid;
								strcpy((char *)msg_buffer,temp1->data->message_buffer);
								current_pcb->message_flag=FALSE;
								temp2->next=temp1->next;
								message_count--;
								return;
							}
						}
					}
				}
			}
			else
			{
				//a message target at the running process and sender by the process matches in the requirement
				if (temp1->data->target_pid==current_pcb->process_id&&source_id==temp1->data->source_pid)
				{
					if (temp1->data->send_length>receive_length)
					{
						*(long *) Z502_ARG6.PTR = ERR_ILLEGAL_MESSAGE_LENGTH;
						return;
					}
					else
					{
						//get the message
						*(long *) Z502_ARG4.PTR = temp1->data->send_length;
						*(long *) Z502_ARG5.PTR = temp1->data->source_pid;
						strcpy((char *)msg_buffer,temp1->data->message_buffer);
						current_pcb->message_flag=FALSE;//don't in need of message any more
						MessageQueue_head=MessageQueue_head->next;
						message_count--;
						return;
					}
				}
				else
				{
					while(temp1->next!=NULL)//search the whole messageQueue
					{
						temp2=temp1;
						temp1=temp1->next;
						//a message target at the running process and sender by the process matches in the requirement
						if (temp1->data->target_pid==current_pcb->process_id&&source_id==temp1->data->source_pid)
						{
							if (temp1->data->send_length>receive_length)
							{
								*(long *) Z502_ARG6.PTR = ERR_ILLEGAL_MESSAGE_LENGTH;
								return;
							}
							else
							{
								//get the message
								*(long *) Z502_ARG4.PTR = temp1->data->send_length;
								*(long *) Z502_ARG5.PTR = temp1->data->source_pid;
								strcpy((char *)msg_buffer,temp1->data->message_buffer);
								current_pcb->message_flag=FALSE;
								temp2->next=temp1->next;
								message_count--;
								return;
							}
						}
					}
				}
			}
		}
		//if we get here which means not get the message we want
		//we have to suspend the running process and waits for a sender to invoke it up
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		current_pcb->message_flag=TRUE;
		suspend_flag[current_pcb->process_id]=0;
		AddToWaitingQueue(current_pcb);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		dispatcher();
	}
}

/*
	os_disk_write
	What: routine responde to disk write calls
	How: if related disk is in use, mark the process to be failed in writting
	suspend the process to WaitingQueue, call dispatcher
	if related disk is free, set to write, mark the process to be succeeded in writting
	suspend the process to WaitingQueue, call dispatcher
*/
void os_disk_write(INT32 disk_id,INT32 sector, char *buffer)
{
	INT32      Temp;
	int i=0,j=0;
	long k=0;

	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
	/* Do the hardware call to put data on disk */
    MEM_WRITE( Z502DiskSetID, &disk_id );
    MEM_READ( Z502DiskStatus, &Temp );
    if ( Temp == DEVICE_FREE )        // Disk hasn't been used - should be free
	{
		MEM_WRITE( Z502DiskSetSector, &sector );
		MEM_WRITE( Z502DiskSetBuffer, (INT32 *)buffer );
		Temp = 1;                        // Specify a write
		MEM_WRITE( Z502DiskSetAction, &Temp );
		Temp = 0;                        // Must be set to 0
		MEM_WRITE( Z502DiskStart, &Temp );

		current_pcb->disk_id_index=disk_id;
		current_pcb->step_index=-1;

		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		
		if (just_for_testing)
		{
			printf("Write and wait...%d %d %d\n",current_pcb->process_id,disk_id,sector);
		}
		
		suspend_flag[current_pcb->process_id]=0;
		AddToWaitingQueue(current_pcb);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		dispatcher();
		return;
	}
	else if ( Temp == DEVICE_IN_USE )
	{
		long       container_b;
		CONTAINER *container_a;
		container_b = ( long )calloc( 1, sizeof( CONTAINER ) );
		container_a = (CONTAINER *) container_b;

		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
		current_pcb->disk_id_index=disk_id;

		current_pcb->step_index=GO_TO_WRITE1;
		current_pcb->Z502_ARG1.VAL=disk_id;
		current_pcb->Z502_ARG2.VAL=sector;

		container_a->int_data[0] =((CONTAINER *)buffer)->int_data[0];
		container_a->int_data[1] =((CONTAINER *)buffer)->int_data[1];
		container_a->int_data[2] =((CONTAINER *)buffer)->int_data[2];
		container_a->int_data[3] =((CONTAINER *)buffer)->int_data[3];
		//printf("Read:%d-%d-%d-%d\n", ((CONTAINER *)buffer)->int_data[0],((CONTAINER *)buffer)->int_data[1],((CONTAINER *)buffer)->int_data[2],((CONTAINER *)buffer)->int_data[3]);
		//printf("Read:%d-%d-%d-%d\n", container_a->int_data[0],container_a->int_data[1],container_a->int_data[2],container_a->int_data[3]);
		
		current_pcb->Z502_ARG3.PTR=container_a;
		//printf("Read:%d-%d-%d-%d\n", ((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[0],((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[1],((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[2],((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[3]);

		if (just_for_testing)
		{
			printf("pid %d failed to write on disk %d %d!!!\n",current_pcb->process_id,disk_id,sector);
		}
		
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		//current_pcb->step_index=GO_TO_STEP2;
		suspend_flag[current_pcb->process_id]=0;
		AddToWaitingQueue(current_pcb);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		dispatcher();
		return;
	}
	else
	{
		printf("some error not processed in os_disk_write!\n");
	} 

	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
}

/*
	os_disk_read
	What: routine responde to disk read calls
	How: if related disk is in use, mark the process to be failed in reading
	suspend the process to WaitingQueue, call dispatcher
	if related disk is free, set to write, mark the process to be succeeded in reading
	suspend the process to WaitingQueue, call dispatcher
	*/
void os_disk_read(INT32 disk_id,INT32 sector, char *buffer)
{
	INT32 Temp;

	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);

	MEM_WRITE( Z502DiskSetID, &disk_id );
	MEM_READ( Z502DiskStatus, &Temp );
    if ( Temp == DEVICE_FREE )        // Disk hasn't been used - should be free
	{
		MEM_WRITE( Z502DiskSetSector, &sector );
		MEM_WRITE( Z502DiskSetBuffer, (INT32 *)buffer );
		Temp = 0;                        // Specify a read
		MEM_WRITE( Z502DiskSetAction, &Temp );
		Temp = 0;                        // Must be set to 0
		MEM_WRITE( Z502DiskStart, &Temp );

		current_pcb->disk_id_index=disk_id;
		current_pcb->step_index=-1;

		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		
		if (just_for_testing)
		{
			printf("Read and wait...%d %d %d\n",current_pcb->process_id,disk_id,sector);
		}
		
		suspend_flag[current_pcb->process_id]=0;
		AddToWaitingQueue(current_pcb);

		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
		dispatcher();
	}
	else if (Temp == DEVICE_IN_USE)
	{
		current_pcb->Z502_ARG3.PTR = ( Z502_ARG *)calloc( 1, sizeof( CONTAINER ) );
	

		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);

		current_pcb->disk_id_index=disk_id;

		current_pcb->step_index=GO_TO_READ1;
		current_pcb->Z502_ARG1.VAL=disk_id;
		current_pcb->Z502_ARG2.VAL=sector;

		if (just_for_testing)
		{
			printf("pid %d failed to read from disk %d %d!!!\n",current_pcb->process_id,disk_id,sector);
		}
		
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);

		suspend_flag[current_pcb->process_id]=0;
		AddToWaitingQueue(current_pcb);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		dispatcher();
	}
	else
	{
		printf("some error not processed in os_disk_read!\n");
	}

	Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
}

/************************************************************************
    SVC
        The beginning of the OS502.  Used to receive software interrupts.
        All system calls come to this point in the code and are to be
        handled by the student written code here.
************************************************************************/
void svc( void ) {
    INT16               call_type;
    static INT16        do_print = 10;
	INT32               Time;
	
    call_type = (INT16)SYS_CALL_CALL_TYPE;
	//if (flag!=16 && flag!=14 && flag!=15 && flag!=19 )
	{
		if ( FULL_PRINT || (do_print > 0) ) {
        printf( "SVC handler: %s %8ld %8ld %8ld %8ld %8ld %8ld\n",
                call_names[call_type], Z502_ARG1.VAL, Z502_ARG2.VAL, 
                Z502_ARG3.VAL, Z502_ARG4.VAL, 
                Z502_ARG5.VAL, Z502_ARG6.VAL );
		//printf("do_print is %d\n",do_print);
        do_print--;
		}
    }
	switch (call_type) {
		//Get time service call
		case SYSNUM_GET_TIME_OF_DAY: //This value is found in syscalls.h
			ZCALL( MEM_READ(Z502ClockStatus, &Time) );
			*(INT32 *)Z502_ARG1.PTR = Time;
			break ;
		//start the clock
		case SYSNUM_SLEEP:
			StartTimer(Z502_ARG1.VAL);
			dispatcher();
			break;//if no break, mistakes will be made.
		//CREATE_PROCESS( process_name, starting_address, initial_priority, &process_id, &error )
		case SYSNUM_CREATE_PROCESS:
			os_make_process1(Z502_ARG1.PTR,Z502_ARG2.PTR,Z502_ARG3.VAL);
			break ;
		// terminate system call
		case SYSNUM_TERMINATE_PROCESS :
			os_end_process();
			break ;
		case SYSNUM_GET_PROCESS_ID :
			os_get_process_id(Z502_ARG1.PTR);
			break;
		case SYSNUM_SUSPEND_PROCESS :
			os_suspend_process(Z502_ARG1.VAL);
			//PrintWaitingQueue();
			break;
		case SYSNUM_RESUME_PROCESS :
			os_resume_process(Z502_ARG1.VAL);
			//PrintWaitingQueue();
			break;
		case SYSNUM_CHANGE_PRIORITY :
			os_change_priority( Z502_ARG1.VAL, Z502_ARG2.VAL);
			break;
		case SYSNUM_SEND_MESSAGE :
			os_send_message(Z502_ARG1.VAL,Z502_ARG2.PTR,Z502_ARG3.VAL);
			break;
		case SYSNUM_RECEIVE_MESSAGE :
			os_receive_message(Z502_ARG1.VAL,Z502_ARG2.PTR,Z502_ARG3.VAL);
			break;
		case SYSNUM_DISK_WRITE :
			os_disk_write(Z502_ARG1.VAL,Z502_ARG2.VAL,(char *)Z502_ARG3.PTR);
			break;
		case SYSNUM_DISK_READ :
			os_disk_read(Z502_ARG1.VAL,Z502_ARG2.VAL,(char *)Z502_ARG3.PTR);
			break;
		default :
			printf("ERROR! call_type not recognized!\n");
			printf("Call_type is - %i\n",call_type );

	}											// End of Switch
}                                               // End of svc 

/************************************************************************
    OS_END_PROCESS
Terminate certain processes.
************************************************************************/
void os_end_process()
	{
		*(long *) Z502_ARG2.PTR = ERR_SUCCESS ;
		if ((Z502_ARG1.VAL==-1)||(Z502_ARG1.VAL==current_pcb->process_id))
		{//If process_id = -1, then terminate self. 
			//if (current_pcb->children!=NULL)
			//{
				//*(long *) Z502_ARG2.PTR = ERR_CHILDREN_PROCESS_NOT_TERMINATED;
				//printf("???");  Still exists bugs!! But no need to care about!
			//	return;
			//}
			//else
			//{
			
				delete_me_process();
			//}
		}
		else if (Z502_ARG1.VAL==-2)
		{//If process_id = -2, then terminate self and any child processes. 
			TQ temp_print;
			//delete_family_process(current_pcb->process_id);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			if (!FULL_PRINT) //print format as jb asked
			{
			CALL( SP_setup_action( SP_ACTION_MODE, "Done" ) );
			CALL( SP_setup( SP_TARGET_MODE, (INT32)current_pcb->process_id ) );
			CALL( SP_setup( SP_TERMINATED_MODE, (INT32)current_pcb->process_id ) );
			CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
			if (ReadyQueue_head!=NULL)
			{
				temp_print=ReadyQueue_head;
				CALL( SP_setup( SP_TERMINATED_MODE, temp_print->data->process_id ) );
				while (temp_print->next!=NULL)
				{
					temp_print=temp_print->next;
					CALL( SP_setup( SP_TERMINATED_MODE, temp_print->data->process_id ) );
				} 
			}
			if (TimerQueue_head!=NULL)
			{
				temp_print=TimerQueue_head;
				CALL( SP_setup( SP_TERMINATED_MODE, temp_print->data->process_id ) );
				while (temp_print->next!=NULL)
				{
					temp_print=temp_print->next;
					CALL( SP_setup( SP_TERMINATED_MODE, temp_print->data->process_id ) );
				} 
			}
			if (WaitingQueue_head!=NULL)
			{
				temp_print=WaitingQueue_head;
				CALL( SP_setup( SP_TERMINATED_MODE, temp_print->data->process_id ) );
				while (temp_print->next!=NULL)
				{
					temp_print=temp_print->next;
					CALL( SP_setup( SP_TERMINATED_MODE, temp_print->data->process_id ) );
				} 
			}
			if ((count++)%15==0)
			{
				CALL( SP_print_header() );
			} 
			CALL( SP_print_line() );
			}
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
			Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1 );
			Z502_HALT();
		}
		else
		{
			if (process_pid[Z502_ARG1.VAL]==-1)
			{
				*(long *) Z502_ARG2.PTR = ERR_NONE_PROCESS_ID;		
				return;
			}
			else
			{
				//if(check_children_process(Z502_ARG1.VAL,current_pcb)!=0)//if exits such a children, returns 0
				//{
					//*(long *) Z502_ARG2.PTR = ERR_NOT_IN_THE_HIERARCHY;
				//}
				//else
				//{
				
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
				delete_the_process(Z502_ARG1.VAL);
				delete_process_id_in_parent(Z502_ARG1.VAL,current_pcb);
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
				Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
				//}
			}
		}
}

/**************************************************************
DELETE_ITSELF
**************************************************************/
void delete_me_process()
	{
		if (current_pcb->process_id==0)
		{
			Z502_HALT();
		}

		while(ReadyQueue_head==NULL)
			CALL(kill_time());
		
		process_pid[current_pcb->process_id]=-1;//make sure of this

		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		if (!FULL_PRINT) //print format as jb asked
		{
		CALL( SP_setup_action( SP_ACTION_MODE, "Done" ) );
		CALL( SP_setup( SP_TARGET_MODE, (INT32)current_pcb->process_id ) );
		CALL( SP_setup( SP_TERMINATED_MODE, (INT32)current_pcb->process_id ) );
		CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
		set_printer();
		if ((count++)%15==0)
		{
			CALL( SP_print_header() );
		} 
		CALL( SP_print_line() );
		}
		current_pcb=RemoveFromReadyQueue();
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
		Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1 );

		ZCALL( Z502_SWITCH_CONTEXT( SWITCH_CONTEXT_KILL_MODE, &(current_pcb->context) ));
}

/************************************************************************
    OS_SWITCH_CONTEXT_COMPLETE
        The hardware, after completing a process switch, calls this routine
        to see if the OS wants to do anything before starting the user 
        process.
************************************************************************/
void os_switch_context_complete( void )
    {
    static INT16        do_print = TRUE;

    if ( do_print == TRUE )
    {
        printf( "os_switch_context_complete  called before user code.\n");
        do_print = FALSE;
    }
	else
	{
		//printf("Hello World!    %d - %d\n",current_pcb->process_id,Z502_PROGRAM_COUNTER);

		if (current_pcb->message_flag==TRUE) //if the process is in need of message try to receive it
		{
			os_receive_message(Z502_ARG1.VAL,Z502_ARG2.PTR,Z502_ARG3.VAL);
		}

		if ( current_pcb->step_index == GO_TO_WRITE2)
		{
			
			//don't need to do anythiing
			current_pcb->step_index=-1; 		
		
		}

		if (current_pcb->step_index == GO_TO_READ2)
		{

			((CONTAINER *)Z502_ARG3.PTR)->int_data[0] =((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[0];
			((CONTAINER *)Z502_ARG3.PTR)->int_data[1] =((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[1];
			((CONTAINER *)Z502_ARG3.PTR)->int_data[2] =((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[2];
			((CONTAINER *)Z502_ARG3.PTR)->int_data[3] =((CONTAINER *)current_pcb->Z502_ARG3.PTR)->int_data[3];

			current_pcb->step_index=-1; 

		}
		
	}
}                               /* End of os_switch_context_complete */

/***********************************************************************
						OS_MAKE_PROCESS
Create a process with quired infromation.

In: process name, starting address, process priority
Out: void
************************************************************************/
void  * os_make_process1(void *a,void *b, int c)
	{
		//PrintReadyQueue();
		*(long *)Z502_ARG4.PTR = -1;  
		*(long *) Z502_ARG5.PTR = ERR_SUCCESS ;//initilization
		//check_priority(c)
		if (c<0||c>100)
			{
				 *(long *) Z502_ARG5.PTR = ERR_ILLEGAL_PRIORITY;
		}
		else if(check_process_name(a)==0)
			{
				//find if such name already exists
				*(long *) Z502_ARG5.PTR = ERR_TWO_THE_SAME_NAME;
		}
		else
			{
				char *container_a;
				void *next_context;
				PCB *pcb1;
				size_t size;
				int flag_repeat;
				
				pcb1 = (PCB *)malloc(sizeof(PCB));
				ZCALL(Z502_MAKE_CONTEXT(&next_context,b, USER_MODE ));
			    pcb1->context=next_context;
				pcb1->priority=c; 
				pcb1->message_flag=FALSE;
				pcb1->step_index=-1;
				flag_repeat=MAX_PROCESS_NUMBER;
				//make sure only produce ceraain number of process
				while (process_pid[count_pcb_id]==0&&flag_repeat>=0)
				{
					count_pcb_id++;
					flag_repeat--;
					if (count_pcb_id==MAX_PROCESS_NUMBER)
					{
						count_pcb_id=0;
					}
				}
				if (flag_repeat>=0)
				{
					pcb1->process_id= count_pcb_id;
					pcb1->disk_id_index= count_pcb_id/2+1;
					*(long *)Z502_ARG4.PTR=count_pcb_id;
					//printf("This new PCB is Number %d\n",count_pcb_id);
					process_pid[count_pcb_id]=0;
					size=strlen((char *)a);
					container_a=(char *)malloc(size+1);
					strcpy(container_a,(const char*)a);
					pcb1->process_name=container_a;
					pcb1->children = NULL;
					//add this pcb id the current parent
					add_child(pcb1->process_id);
					
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
					AddToReadyQueue(pcb1); //After creation, the process is added to ReadyQueue.
					
					if (!FULL_PRINT) //print format as jb asked
					{
					CALL( SP_setup_action( SP_ACTION_MODE, "Create" ) );
					CALL( SP_setup( SP_TARGET_MODE, (INT32)pcb1->process_id ) );
					CALL( SP_setup( SP_NEW_MODE, (INT32)pcb1->process_id ) );
					CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
					set_printer();
					if ((count++)%15==0)
					{
						CALL( SP_print_header() );
					} 
					CALL( SP_print_line() );
					}

					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
					Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1 );
				}
				else
				{
					*(long *) Z502_ARG5.PTR = ERR_EXCEED_MAX_PROCESS_NUMBER;
				}
		}		
}

/**************************************************************
	ADD_CHILD
//add this pcb id the current parent, whcih is current pcb
In: id of the child
***************************************************************/				
void add_child(int a)
	{	
		struct offspring *temp,*temp1;
		temp=(struct offspring *)malloc(sizeof(struct offspring ));
		if (current_pcb->children==NULL) //TimerQueue=NULL a huge mistake
		{
			temp->child_id=a;
			temp->next_child=NULL;
			current_pcb->children=temp;
		}
		else 
		{			
			temp=current_pcb->children ;
			while(temp->next_child!=NULL)
				temp=temp->next_child;

			temp1=(struct offspring *)malloc(sizeof(struct offspring ));
			temp->next_child = temp1 ;
			temp = temp->next_child ;
			temp->child_id = a;
			temp->next_child = NULL;
		}
		//printf ("We have added PCB %d to its parent PCB %d.\n",a,current_pcb->process_id);
}

/***********************************************************************
OS_MAKE_PROCESS
	We come to os_create_process, a routine YOU write.  Here we ask the 
	hardware for  a context, create the PCB, and then call Z502_SWITCH_CONTEXT.
************************************************************************/
PCB * os_make_process(int flag)
	{
		PCB *pcb1;
		void *next_context;
		pcb1 = (PCB *)malloc(sizeof(PCB));
		//different flage value, different routine we start
		if (flag == 0){
			FULL_PRINT=TRUE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test0, USER_MODE ));
		}//ask hardware for context
		else if (flag == 1){
			FULL_PRINT=TRUE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1a, USER_MODE ));
		}
		else if (flag == 2){
			FULL_PRINT=TRUE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1b, USER_MODE ));
		}
		else if (flag == 3){
			FULL_PRINT=FALSE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1c, USER_MODE ));
		}
		else if (flag == 4){
			FULL_PRINT=FALSE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1d, USER_MODE ));
		}
		else if (flag == 5){
			FULL_PRINT=TRUE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1e, USER_MODE ));
		}
		else if (flag == 6){
			FULL_PRINT=FALSE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1f, USER_MODE ));
		}
		else if (flag == 7){
			FULL_PRINT=TRUE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1g, USER_MODE ));
		}
		else if (flag == 8){
			FULL_PRINT=FALSE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1h, USER_MODE ));
		}
		else if (flag == 9){
			FULL_PRINT=TRUE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1i, USER_MODE ));
		}
		else if (flag == 10){
			FULL_PRINT=FALSE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1j, USER_MODE ));
		}
		else if (flag == 11){
			FULL_PRINT=TRUE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test1k, USER_MODE ));
		}
		else if (flag == 12){
			FULL_PRINT=TRUE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2a, USER_MODE ));
		}
		else if (flag == 13){
			FULL_PRINT=TRUE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2b, USER_MODE ));
		}
		else if (flag == 14){
			FULL_PRINT=FALSE;
			scheduler_full_print=!FULL_PRINT;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2c, USER_MODE ));
		}
		else if (flag == 15){
			FULL_PRINT=FALSE;
			scheduler_full_print=FALSE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2d, USER_MODE ));
		}
		else if (flag == 16){
			FULL_PRINT=FALSE;
			scheduler_full_print=FALSE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2e, USER_MODE ));
		}
		else if (flag == 17){
			FULL_PRINT=FALSE;
			scheduler_full_print=FALSE;
			obvisouly_print=FALSE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2f, USER_MODE ));
		}
		else if (flag == 18){
			FULL_PRINT=TRUE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2g, USER_MODE ));
		}
		else if (flag == 19){
			FULL_PRINT=TRUE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2h, USER_MODE ));
		}
		else if (flag == 20){
			FULL_PRINT=TRUE;
			ZCALL(Z502_MAKE_CONTEXT(&next_context,(void *)test2gx, USER_MODE ));
		}
		else{ 
			printf ("Input Wrong Arguments");
		}
		pcb1->context=next_context;
		pcb1->priority=10; 
		pcb1->message_flag=FALSE;
		pcb1->step_index=-1;
		pcb1->disk_id_index=-1;
		pcb1->children = NULL;
		
		while (process_pid[count_pcb_id]==0)
				{
					count_pcb_id++;
					if (count_pcb_id==MAX_PROCESS_NUMBER)
					{
						count_pcb_id=0;
					}
				}
		pcb1->process_id= count_pcb_id ;//strat from zero, 
		process_pid[count_pcb_id]=0;

		pcb1->process_name=(char *)"the_first_pcb";
		return pcb1;      
}

/************************************************************************
    OS_INIT
        This is the first routine called after the simulation begins.  This 
        is equivalent to boot code.  All the initial OS components can be
        defined and initialized here.
************************************************************************/
void    os_init( void )
    {
    void                *next_context;
    INT32               i;
	
	
	//初始化TimerQueue
	TimerQueue_head=NULL;
	ReadyQueue_head=NULL;
	WaitingQueue_head=NULL;
	DiskQueue_head=NULL;

	MessageQueue_head=NULL;

	temp=(struct Node *)malloc(sizeof(struct Node ));//如果没有这一句 下面temp根本不能被赋值

	for (i = 0; i < MAX_PROCESS_NUMBER; i++)
	{
		process_pid[i]=-1;
		suspend_flag[i]=-1;
	}

	
	//for (i = 0; i < 1; i++)
	for (i = 0; i < PHYS_MEM_PGS; i++)
	{
		AddToFreeFrameList((short) i);
		memory_organizer[i]=NULL;
	}

    /* Demonstrates how calling arguments are passed thru to here       */

    printf( "Program called with %d arguments:", CALLING_ARGC );
    for ( i = 0; i < CALLING_ARGC; i++ )
        printf( " %s", CALLING_ARGV[i] );
    printf( "\n" );
    printf( "Calling with argument 'sample' executes the sample program.\n" );

    /*          Setup so handlers will come to code in base.c           */

    TO_VECTOR[TO_VECTOR_INT_HANDLER_ADDR]   = (void *)interrupt_handler;
    TO_VECTOR[TO_VECTOR_FAULT_HANDLER_ADDR] = (void *)fault_handler;
    TO_VECTOR[TO_VECTOR_TRAP_HANDLER_ADDR]  = (void *)svc;

    /*  Determine if the switch was set, and if so go to demo routine.  */

    if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "sample" ) == 0 ) )
        {
        ZCALL( Z502_MAKE_CONTEXT( &next_context, 
                                        (void *)sample_code, KERNEL_MODE ));
        ZCALL( Z502_SWITCH_CONTEXT( SWITCH_CONTEXT_KILL_MODE, &next_context ));
    }                   /* This routine should never return!!           */

    /*  This should be done by a "os_make_process" routine, so that
        test0 runs on a process recognized by the operating system.    */
	//for different argument, map to different values in flag
	
	if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test0" ) == 0 ) )
        {
        flag = 0;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1a" ) == 0 ) )
        {
        flag = 1;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1b" ) == 0 ) )
        {
        flag = 2;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1c" ) == 0 ) )
        {
        flag = 3;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1d" ) == 0 ) )
        {
        flag = 4;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1e" ) == 0 ) )
        {
        flag = 5;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1f" ) == 0 ) )
        {
        flag = 6;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1g" ) == 0 ) )
        {
        flag = 7;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1h" ) == 0 ) )
        {
        flag = 8;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1i" ) == 0 ) )
        {
        flag = 9;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1j" ) == 0 ) )
        {
        flag = 10;
    }
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test1k" ) == 0 ) )
        {
        flag = 11;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2a" ) == 0 ) )
        {
        flag = 12;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2b" ) == 0 ) )
        {
        flag = 13;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2c" ) == 0 ) )
        {
        flag = 14;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2d" ) == 0 ) )
        {
        flag = 15;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2e" ) == 0 ) )
        {
        flag = 16;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2f" ) == 0 ) )
        {
        flag = 17;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2g" ) == 0 ) )
        {
        flag = 18;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2h" ) == 0 ) )
        {
        flag = 19;
	}
	else if (( CALLING_ARGC > 1 ) && ( strcmp( CALLING_ARGV[1], "test2gx" ) == 0 ) )
        {
        flag = 20;
	}
	else
	{
		flag=0;
	}

	pcb=os_make_process(flag );
	//printf("\n\n This is test %d\n",pcb->process_id);
	current_pcb = pcb ;
	if (!FULL_PRINT) //print format as jb asked
	{
	CALL( SP_setup_action( SP_ACTION_MODE, "Create" ) );
	CALL( SP_setup( SP_TARGET_MODE, (INT32)current_pcb->process_id ) );
	CALL( SP_setup( SP_NEW_MODE, current_pcb->process_id ) );
	CALL( SP_setup( SP_RUNNING_MODE, current_pcb->process_id ) );
	CALL( SP_print_header() );
	CALL( SP_print_line() );
	}
	ZCALL( Z502_SWITCH_CONTEXT( SWITCH_CONTEXT_KILL_MODE, &(current_pcb->context) ));
}                                               /* End of os_init       */