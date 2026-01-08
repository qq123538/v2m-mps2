#include "cmsis_os2.h"
#include "logger.h"
#include "macro.h"

/* Queue handles */
osMessageQueueId_t g_test_queue;
osMessageQueueId_t g_long_name_queue;

/* Task IDs */
osThreadId_t g_receiver_task;
osThreadId_t g_sender_task;

/* Receiver Task: Waits for data from the queue */
static void queue_receiver_task( void* argument )
{
    (void)argument;
    uint32_t rx_data;
    osStatus_t status;

    for ( ;; )
    {
        /* Block forever until data is available.
           This should make this task appear in the 'RxWait' list of the queue
           when the queue is empty. */
        // LOG("RxTask: Waiting for data...");
        status = osMessageQueueGet( g_test_queue, &rx_data, NULL, osWaitForever );

        if ( status == osOK )
        {
            LOG( "RxTask: Received data: 0x%X", rx_data );
        }
        else
        {
            LOG( "RxTask: Error receiving data: %d", status );
        }
    }
}

/* Sender Task: Sends data to the queue periodically */
static void queue_sender_task( void* argument )
{
    (void)argument;
    uint32_t tx_data = 0x12345678;

    for ( ;; )
    {
        /* Send data. Wait up to 100ms if queue is full. */
        // LOG("TxTask: Sending 0x%X...", tx_data);
        osStatus_t stat = osMessageQueuePut( g_test_queue, &tx_data, 0, 100 );

        if ( stat == osOK )
        {
            LOG( "TxTask: Sent 0x%X", tx_data );
        }
        else if ( stat == osErrorTimeout )
        {
            LOG( "TxTask: Queue full, timeout sending 0x%X", tx_data );
        }

        /* Delay to keep the receiver blocked most of the time */
        osDelay( 5000 );
        tx_data++;
    }
}

void create_queue_test( void )
{
    /* 1. Standard Test Queue */
    const osMessageQueueAttr_t test_queue_attr = { .name = "TestQueue" };
    g_test_queue = osMessageQueueNew( 10, sizeof( uint32_t ), &test_queue_attr );

    /* 2. Long Named Queue */
    const osMessageQueueAttr_t long_name_attr = { .name = "LongNameQueueForDebugging" };
    g_long_name_queue = osMessageQueueNew( 5, sizeof( uint32_t ), &long_name_attr );

    /* Create Tasks */
    const osThreadAttr_t rx_task_attr = { .name = "QueueReceiver",
                                          .stack_size = 2048,
                                          .priority = osPriorityNormal };
    g_receiver_task = osThreadNew( queue_receiver_task, NULL, &rx_task_attr );
    CUSTOM_ASSERT( g_receiver_task != NULL );

    const osThreadAttr_t tx_task_attr = { .name = "QueueSender",
                                          .stack_size = 2048,
                                          .priority = osPriorityBelowNormal };
    g_sender_task = osThreadNew( queue_sender_task, NULL, &tx_task_attr );
    CUSTOM_ASSERT( g_sender_task != NULL );
}