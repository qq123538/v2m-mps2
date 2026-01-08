#include "cmsis_os2.h"
#include "logger.h"
#include "macro.h"

static void task_1( void* argument )
{
    ( void )argument;

    for ( ;; )
    {
        LOG( "Hello from Task 1" );
        osDelay( 1000 );
    }
}

static void task_2( void* argument )
{
    ( void )argument;

    for ( ;; )
    {
        LOG( "Hello from Task 2" );
        osDelay( 2000 );
    }
}

void create_tasks_test( void )
{
    const osThreadAttr_t task_1_attr = { .name = "Task 1",
                                         .stack_size = 2 * 1024,
                                         .priority = osPriorityBelowNormal };

    CUSTOM_ASSERT( osThreadNew( task_1, NULL, &task_1_attr ) != NULL );

    const osThreadAttr_t task_2_attr = { .name = "Task 2",
                                         .stack_size = 2 * 1024,
                                         .priority = osPriorityBelowNormal };

    CUSTOM_ASSERT( osThreadNew( task_2, NULL, &task_2_attr ) != NULL );
}