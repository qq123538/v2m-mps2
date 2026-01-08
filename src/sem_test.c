#include "cmsis_os2.h"
#include "logger.h"
#include "macro.h"

/* Semaphore/Mutex handles */
osSemaphoreId_t g_binary_sem;
osSemaphoreId_t g_counting_sem;
osMutexId_t g_normal_mutex;
osMutexId_t g_recursive_mutex;

/* Task that holds the mutex to show "Holder" in GDB */
static void mutex_holder_task( void* argument )
{
    ( void )argument;

    for ( ;; )
    {
        /* Take the mutex */
        LOG( "MutexHolder: Attempting to acquire NormalMutex..." );
        if ( osMutexAcquire( g_normal_mutex, osWaitForever ) == osOK )
        {
            LOG( "MutexHolder: Acquired NormalMutex. Holding for 30s." );

            /* Hold it for a long time (30 seconds) to allow GDB inspection */
            osDelay( 30000 );

            /* Release it */
            LOG( "MutexHolder: Releasing NormalMutex." );
            osMutexRelease( g_normal_mutex );
        }

        /* Small delay before taking it again */
        osDelay( 100 );
    }
}

/* Task for recursive mutex testing */
static void recursive_task( void* argument )
{
    ( void )argument;

    for ( ;; )
    {
        LOG( "RecurseTask: Acquiring RecursiveMutex (Depth 1)..." );
        if ( osMutexAcquire( g_recursive_mutex, osWaitForever ) == osOK )
        {
            /* Take it a second time (recursive) */
            LOG( "RecurseTask: Acquiring RecursiveMutex (Depth 2)..." );
            if ( osMutexAcquire( g_recursive_mutex, osWaitForever ) == osOK )
            {
                LOG( "RecurseTask: Holding RecursiveMutex at Depth 2 for 10s." );
                osDelay( 10000 );

                osMutexRelease( g_recursive_mutex );
                LOG( "RecurseTask: Released Depth 2." );
            }
            osMutexRelease( g_recursive_mutex );
            LOG( "RecurseTask: Released Depth 1." );
        }
        osDelay( 1000 );
    }
}

void create_sem_test( void )
{
    /* 1. Binary Semaphore */
    const osSemaphoreAttr_t bin_sem_attr = { .name = "BinarySem" };
    g_binary_sem = osSemaphoreNew( 1, 0, &bin_sem_attr );

    /* 2. Counting Semaphore (Max 10, Initial 5) */
    const osSemaphoreAttr_t count_sem_attr = { .name = "CountingSem" };
    g_counting_sem = osSemaphoreNew( 10, 5, &count_sem_attr );

    /* 3. Normal Mutex */
    const osMutexAttr_t mutex_attr = { .name = "NormalMutex" };
    g_normal_mutex = osMutexNew( &mutex_attr );

    /* 4. Recursive Mutex */
    const osMutexAttr_t recursive_attr = { .name = "RecursiveMutex",
                                           .attr_bits = osMutexRecursive };
    g_recursive_mutex = osMutexNew( &recursive_attr );

    /* Create tasks to occupy the mutexes */
    const osThreadAttr_t holder_attr = { .name = "MutexHolder",
                                         .stack_size = 2048,
                                         .priority = osPriorityNormal };
    CUSTOM_ASSERT( osThreadNew( mutex_holder_task, NULL, &holder_attr ) != NULL );

    const osThreadAttr_t recurse_attr = { .name = "RecurseTask",
                                          .stack_size = 2048,
                                          .priority = osPriorityBelowNormal };
    CUSTOM_ASSERT( osThreadNew( recursive_task, NULL, &recurse_attr ) != NULL );
}