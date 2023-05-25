/* This is a small demo of the high-performance ThreadX kernel.  It includes examples of eight
   threads of different priorities, using a message queue, semaphore, mutex, event flags group, 
   byte pool, and block pool.  */

#include "tx_api.h"

#define DEMO_STACK_SIZE         400
#define DEMO_BYTE_POOL_SIZE     1024
#define DEMO_BLOCK_POOL_SIZE    10
#define DEMO_QUEUE_SIZE         10


/* Define the ThreadX object control blocks...  */

TX_THREAD               thread_1;
TX_THREAD               thread_2;
TX_QUEUE                queue_1;
TX_QUEUE                queue_2;
TX_BYTE_POOL            byte_pool_0;
UCHAR                   memory_area[DEMO_BYTE_POOL_SIZE];


/* Define the counters used in the demo application...  */

ULONG                   thread_1_counter;
ULONG                   thread_1_messages_sent;
ULONG                   thread_1_messages_received;
ULONG                   thread_2_counter;
ULONG                   thread_2_messages_sent;
ULONG                   thread_2_messages_received;


/* Define thread prototypes.  */

void    thread_1_entry(ULONG thread_input);
void    thread_2_entry(ULONG thread_input);


/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

CHAR    *pointer = TX_NULL;
	UINT status;

    /* Create a byte memory pool from which to allocate the thread stacks.  */
	status = tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area, DEMO_BYTE_POOL_SIZE);
    if(status != TX_SUCCESS) while(1);

    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */

    /* Allocate the stack for thread 1.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if(status != TX_SUCCESS) while(1);

    /* Create threads 1 and 2. These threads pass information through a ThreadX 
       message queue.  It is also interesting to note that these threads have a time
       slice.  */
    status = tx_thread_create(&thread_1, "thread 1", thread_1_entry, 1,
            pointer, DEMO_STACK_SIZE, 
            16, 16, TX_NO_TIME_SLICE, TX_AUTO_START);
    if(status != TX_SUCCESS) while(1);

    /* Allocate the stack for thread 2.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if(status != TX_SUCCESS) while(1);

    status = tx_thread_create(&thread_2, "thread 2", thread_2_entry, 2,
            pointer, DEMO_STACK_SIZE, 
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if(status != TX_SUCCESS) while(1);

    /* Allocate the message queue.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    if(status != TX_SUCCESS) while(1);

    /* Create the message queue for threads 1.  */
    status = tx_queue_create(&queue_1, "queue 1", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));
    if(status != TX_SUCCESS) while(1);

    /* Allocate the message queue.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    if(status != TX_SUCCESS) while(1);

    /* Create the message queue for threads 2.  */
    status = tx_queue_create(&queue_2, "queue 2", TX_2_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));
    if(status != TX_SUCCESS) while(1);

    /* Release the block back to the pool.  */
    status = tx_byte_release(pointer);
    if(status != TX_SUCCESS) while(1);
}


void    thread_1_entry(ULONG thread_input)
{
	ULONG   received_message;
	UINT    status;

    /* This thread simply sends messages to a queue shared by thread 2.  */
    while(1) {
        /* Retrieve a message from the queue.  */
        status = tx_queue_receive(&queue_1, &received_message, TX_WAIT_FOREVER);
        /* Check completion status and make sure the message is what we expected.  */
        if ((status != TX_SUCCESS) || (received_message != thread_1_messages_received)) while(1);
        /* Otherwise, all is okay.  Increment the received message count.  */
        thread_1_messages_received++;

        /* Increment the thread counter.  */
        thread_1_counter++;

        /* Send message to queue 2.  */
        status =  tx_queue_send(&queue_2, &thread_1_messages_sent, TX_WAIT_FOREVER);
        /* Check completion status.  */
        if (status != TX_SUCCESS) while(1);
        /* Increment the message sent.  */
        thread_1_messages_sent++;
    }
}


void    thread_2_entry(ULONG thread_input)
{
	ULONG   received_message;
	UINT    status;

    /* Send message to queue 1.  */
    status =  tx_queue_send(&queue_1, &thread_2_messages_sent, TX_WAIT_FOREVER);
    /* Check completion status.  */
    if (status != TX_SUCCESS) while(1);
    /* Increment the message sent.  */
    thread_2_messages_sent++;

    /* This thread retrieves messages placed on the queue by thread 1.  */
    while(1) {
        /* Retrieve a message from the queue.  */
        status = tx_queue_receive(&queue_2, &received_message, TX_WAIT_FOREVER);
        /* Check completion status and make sure the message is what we expected.  */
        if ((status != TX_SUCCESS) || (received_message != thread_2_messages_received)) while(1);
        /* Otherwise, all is okay.  Increment the received message count.  */
        thread_2_messages_received++;

        /* Increment the thread counter.  */
        thread_2_counter++;

        /* Send message to queue 1.  */
        status =  tx_queue_send(&queue_1, &thread_2_messages_sent, TX_WAIT_FOREVER);
        /* Check completion status.  */
        if (status != TX_SUCCESS) while(1);
        /* Increment the message sent.  */
        thread_2_messages_sent++;
    }
}
