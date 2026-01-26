#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "logger.h"
#include "EventRecorder.h"

extern int stdout_init( void );
extern void create_tasks_test( void );
extern void create_timer_test( void );
extern void create_queue_test( void );
extern void create_sem_test( void );

int main( void )
{
    // Abstracted System Initialization
    SystemCoreClockUpdate();
    stdout_init();
    EventRecorderInitialize( EventRecordAll, 1 );

    // Initialize the RTOS Kernel
    osKernelInitialize();

    // Initialize the Async Logger
    logger_init();

    // Create various test tasks and objects
    create_tasks_test();
    create_timer_test();
    create_queue_test();
    create_sem_test();

    // Start the RTOS Kernel
    osKernelStart();

    return 0;
}
