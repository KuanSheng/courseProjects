#define          USER
#include         "global.h"
#include         "syscalls.h"
#include         "z502.h"
#include         "protos.h"

#include         "stdio.h"
#include         "string.h"
#include         "stdlib.h"
#include         "math.h"

#include		 "my_includes.h"

extern char          MEMORY[];     //to be deleted  

extern INT16    Z502_PROGRAM_COUNTER;
extern INT32    SYS_CALL_CALL_TYPE;
extern Z502_ARG Z502_ARG1;
extern Z502_ARG Z502_ARG2;
extern Z502_ARG Z502_ARG3;
extern Z502_ARG Z502_ARG4;
extern Z502_ARG Z502_ARG5;
extern Z502_ARG Z502_ARG6;
extern long     Z502_REG_1;
extern long     Z502_REG_2;
extern long     Z502_REG_3;
extern long     Z502_REG_4;
extern long     Z502_REG_5;
extern long     Z502_REG_6;
extern long     Z502_REG_7;
extern long     Z502_REG_8;
extern long     Z502_REG_9;
extern INT16    Z502_MODE;

#define           MOST_FAVORABLE_PRIORITY                       1

void    test2h( void )
    {
    static INT32         trash;

    while(1)
        {
        SELECT_STEP
            {
           STEP( 0 )
                GET_PROCESS_ID( "", &Z502_REG_4, &Z502_REG_5 );
                
           STEP( 1 )
                printf( "\n\nRelease %s:Test 2d: Pid %ld\n", 
                                                CURRENT_REL, Z502_REG_4 );
                CHANGE_PRIORITY( -1, MOST_FAVORABLE_PRIORITY,
                                                        &Z502_REG_9);
           STEP( 2 )
                CREATE_PROCESS( "first", test2i, 5, &trash, &Z502_REG_5 );
                
           STEP( 3 )
                CREATE_PROCESS( "second", test2i, 5, &trash, &Z502_REG_5 );
                
           STEP( 4 )
                //CREATE_PROCESS( "third", test2i, 7, &trash, &Z502_REG_5 );
				break;

           STEP( 5 )
                //CREATE_PROCESS( "fourth", test2i, 7, &trash, &Z502_REG_5 );
				break;
                
           STEP( 6 )
               // CREATE_PROCESS( "fifth", test2i, 7, &trash, &Z502_REG_5 );
			   break;
                
           STEP( 7 )
                SLEEP ( 2000000 ); //200000
                
           STEP( 8 )
                TERMINATE_PROCESS( -1, &Z502_REG_5 );
                

        }                                       /* End of SELECT    */
    }                                           /* End of while     */
}                                               /* End of test2d    */

#define         STEP_SIZE               VIRTUAL_MEM_PGS/(2 * PHYS_MEM_PGS )
#define         DISPLAY_GRANULARITY2e     16 * STEP_SIZE
void    test2i( void )
    {

     while(1)
        {
        SELECT_STEP
            {
           STEP( 0 )
                GET_PROCESS_ID( "", &Z502_REG_4, &Z502_REG_9 );
                
           STEP( 1 )
                printf( "\n\nRelease %s:Test 2e: Pid %ld\n", 
                                                CURRENT_REL, Z502_REG_4 );
                break;
           STEP( 2 )
                Z502_REG_3 = PGSIZE * Z502_REG_7;       /* Generate address*/
                Z502_REG_1 = Z502_REG_3 + Z502_REG_4;   /* Generate data */
                MEM_WRITE( Z502_REG_3, &Z502_REG_1 );   /* Write the data */
                
           STEP( 3 )
                MEM_READ( Z502_REG_3, &Z502_REG_2 );    /* Read back data */
                
           STEP( 4 )
                if ( Z502_REG_7 % DISPLAY_GRANULARITY2e == 0 )
                  printf("PID= %ld  address= %ld   written= %ld   read= %ld\n",
                        Z502_REG_4, Z502_REG_3, Z502_REG_1, Z502_REG_2 );
                if (Z502_REG_2 != Z502_REG_1 )          /* Written = read? */
                    printf( "AN ERROR HAS OCCURRED.\n" );
                Z502_REG_7 += STEP_SIZE;
                if ( Z502_REG_7 < VIRTUAL_MEM_PGS )
                    GO_NEXT_TO( 2 )                     /* Go write/read */
                break;


                /* Now read back the data we've written and paged */

           STEP( 5 )
                printf( "Reading back data: test 2e, PID %ld.\n", 
                                                        Z502_REG_4 );
                Z502_REG_7 = 0;
                break;
           STEP( 6 )
                Z502_REG_3 = PGSIZE * Z502_REG_7;       /* Generate address*/
                Z502_REG_1 = Z502_REG_3 + Z502_REG_4;   /* Data expected  */
                MEM_READ( Z502_REG_3, &Z502_REG_2 );    /* Read back data */
                
           STEP( 7 )
                if ( Z502_REG_7 % DISPLAY_GRANULARITY2e == 0 )
                  printf("PID= %ld  address= %ld   written= %ld   read= %ld\n",
                        Z502_REG_4, Z502_REG_3, Z502_REG_1, Z502_REG_2 );
                if (Z502_REG_2 != Z502_REG_1 )          /* Written = read? */
                    printf( "AN ERROR HAS OCCURRED.\n" );
                Z502_REG_7 += STEP_SIZE;
                GO_NEXT_TO( 6 )                         /* Go write/read */
                break;
			STEP( 8 )
                TERMINATE_PROCESS( -1, &Z502_REG_9 );
        }                                       /* End of SELECT    */
    }                                           /* End of while     */
}  