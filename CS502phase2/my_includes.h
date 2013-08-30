/***************************************************************************

  my_includes.h

      This include file is used by the base.c
	  
	  PS:
		The whole system is runned in Windows7.
		The code is compiled and tested in Microsoft Visual Studio 2012.

      Revision History:
        1.0 9/19/2012:        first release

****************************************************************************/
#define			MAX_PROCESS_NUMBER				30
#define			MAX_LEGAL_MESSAGE_LENGTH        64
#define			MAX_MESSAGE_NUMBER				10

		/* Supplement Definition of return codes.  				 */
#define			ERR_ILLEGAL_PRIORITY					-1L
#define			ERR_TWO_THE_SAME_NAME					-2L
#define			ERR_NONE_PROCESS_ID					    -3L
#define			ERR_CHILDREN_PROCESS_NOT_TERMINATED		-4L
#define			ERR_NOT_IN_THE_HIERARCHY				-5L
#define			ERR_EXCEED_MAX_PROCESS_NUMBER 			-6L
#define			ERR_SUSPEND_ONESELF	        			-7L
#define			ERR_RESUME_ONESELF	        			-8L
#define			ERR_ALREADY_SUSPENDED        			-9L
#define			ERR_ALREADY_RESUMED		        		-10L
#define			ERR_ILLEGAL_MESSAGE_LENGTH				-11L
#define			ERR_EXCEED_MAX_MESSAGE_NUMBER 			-12L

/* Definition of parameters on locks*/
#define                  DO_LOCK                     1
#define                  DO_UNLOCK                   0
#define                  SUSPEND_UNTIL_LOCKED        TRUE
#define                  DO_NOT_SUSPEND              FALSE

#define                  GO_TO_STEP2                  2
#define                  GO_TO_STEP3                  3
#define                  GO_TO_STEP6                  6
#define                  GO_TO_WRITE1                 0
#define                  GO_TO_WRITE2                 1
#define                  GO_TO_READ1                  4
#define                  GO_TO_READ2                  5


INT32			         LockResult1;
INT32			         LockResult2;
INT32			         LockResult3;
INT32			         LockResult4;
INT32			         LockResult5;

struct offspring
{
	int child_id;
	struct offspring *next_child;
};

//structure of PCB
typedef struct
{
	void *context;
	int process_id;
	int priority;
	char *process_name;
	struct offspring  *children;
	BOOL message_flag;
	short step_index;
	INT16 disk_id_index;
	long wake_up_time;
	Z502_ARG Z502_ARG1;
	Z502_ARG Z502_ARG2;
	Z502_ARG Z502_ARG3;
	BOOL *is_dirty_pointer;
}PCB;

//structure of Queues containing the PCBs
struct Node
{
		PCB 		*data;
		struct Node   *next;
};
typedef struct Node *TQ;

////structure of Message
typedef struct MG
{
	INT32 target_pid;
	INT32 source_pid;
	INT32 send_length;
	char message_buffer[MAX_LEGAL_MESSAGE_LENGTH];
}Message;

//structure of Queues containing the Messages
struct NodeM
{
	Message 		*data;
	struct NodeM   *next;
};
typedef struct NodeM *MQ;

//Disk Operation Lock
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+4, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult5);
//ReadyQueue lock
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult1);
//TimerQueue Lock
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+1, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult2);
//WaitingQueue Lock
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+2, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult3);
//suspend_flag[] Lock
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_LOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);
//Z502_READ_MODIFY( MEMORY_INTERLOCK_BASE+3, DO_UNLOCK, SUSPEND_UNTIL_LOCKED, &LockResult4);


typedef struct
{
	int pid;
	short index;
}FI;

typedef struct DKQ
{
	INT16 disk_id;
	int pid;
}DiskQ;

struct NodeD
{
		DiskQ *data;
		struct NodeD   *next;
};
typedef struct NodeD *DQ;

/**
debug
while ( time_of_event < 0 || time_of_event > (INT32)current_simulation_time )
GetLock ( HardwareLock , "hardware_interrupt-2");
*/

struct NodeF
{
	short 		frame_number;
	struct NodeF   *next;
};
typedef struct NodeF *FF;

typedef         union
		{
		char        char_data[ PGSIZE ];
		UINT32      int_data[ PGSIZE/ sizeof(int) ];
		}CONTAINER;