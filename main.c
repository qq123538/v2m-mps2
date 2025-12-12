#include "RTE_Components.h"
#include CMSIS_device_header
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

extern int stdout_init( void );

static void vTask1( void* pvParameters )
{
    (void)pvParameters;

    for ( ;; )
    {
        printf( "Hello from Task 1\r\n" );
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}

static void vTask2( void* pvParameters )
{
    (void)pvParameters;

    for ( ;; )
    {
        printf( "Hello from Task 2\r\n" );
        vTaskDelay( pdMS_TO_TICKS( 10000 ) );
    }
}

#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )

void vApplicationStackOverflowHook( TaskHandle_t xTask, char* pcTaskName )
{
    /* Check pcTaskName for the name of the offending task,
     * or pxCurrentTCB if pcTaskName has itself been corrupted. */
    (void)xTask;
    (void)pcTaskName;
}

#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW > 0 ) */

int main( void )
{
    SystemCoreClockUpdate();
    stdout_init();

    printf( "Hello, World!\n" );

    xTaskCreate( vTask1, "Task 1", 200, NULL, tskIDLE_PRIORITY + 1, NULL );
    xTaskCreate( vTask2, "Task 2", 200, NULL, tskIDLE_PRIORITY + 1, NULL );
    vTaskStartScheduler();
    // Will not get here unless a task calls vTaskEndScheduler ()
    return 0;
}
