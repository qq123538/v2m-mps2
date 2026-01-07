#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "logger.h"
#include <stdio.h>

extern int stdout_init( void );
extern void create_timer_test(void);

static void vTask1( void* pvParameters )
{
    (void)pvParameters;

    for ( ;; )
    {
        LOG( "Hello from Task 1\r\n" );
        osDelay( 1000 );
    }
}

static void vTask2( void* pvParameters )
{
    (void)pvParameters;

    for ( ;; )
    {
        LOG( "Hello from Task 2\r\n" );
        osDelay( 2000 );
    }
}

int main( void )
{
    SystemCoreClockUpdate();
    stdout_init();

    osKernelInitialize();

    // Initialize the Async Logger
    logger_init();

    const osThreadAttr_t task1_attr = { .name = "Task 1",
                                        .stack_size = 2 * 1024,
                                        .priority = osPriorityBelowNormal };

    osThreadNew( vTask1, NULL, &task1_attr );

    const osThreadAttr_t task2_attr = { .name = "Task 2",
                                        .stack_size = 2 * 1024,
                                        .priority = osPriorityBelowNormal };

    osThreadNew( vTask2, NULL, &task2_attr );
    create_timer_test();
    osKernelStart();

    return 0;
}
