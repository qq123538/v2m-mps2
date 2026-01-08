#include "logger.h"
#include "cmsis_compiler.h"
#include "cmsis_os2.h"
#include <stdarg.h>
#include <stdio.h>

/* Import low-level character output function */
extern int stdout_putchar( int ch );

/* Configuration */
#define LOGGER_BUFFER_SIZE 2048 /* Total size of the ring buffer */
#define LOGGER_MAX_LINE                                                                            \
    128 /* Max length of a single log line (stack buffer)                                          \
         */

/* Static Resources */
static uint8_t g_logBuffer[LOGGER_BUFFER_SIZE];
static volatile uint32_t g_head = 0; /* Write Index */
static volatile uint32_t g_tail = 0; /* Read Index */
static osThreadId_t loggerThread;

/*
 * The Logger Task
 * Wakes up on flag, drains ring buffer to UART
 */
static void logger_task( void* argument )
{
    (void)argument;

    for ( ;; )
    {
        // Wait for data signal (do not clear flags automatically if using 'any',
        // but here we just wait)
        osThreadFlagsWait( 0x01, osFlagsWaitAny, osWaitForever );

        // Drain the buffer
        while ( g_tail != g_head )
        {
            uint8_t ch = g_logBuffer[g_tail];

            // Advance tail (atomic read is safe, only this task modifies tail)
            g_tail = ( g_tail + 1 ) % LOGGER_BUFFER_SIZE;

            stdout_putchar( ch );
        }
    }
}

/*
 * Initialize the Logger
 */
void logger_init( void )
{
    g_head = 0;
    g_tail = 0;

    const osThreadAttr_t logger_attr = { .name = "LoggerTask",
                                         .stack_size = 512,
                                         .priority = osPriorityLow };

    loggerThread = osThreadNew( logger_task, NULL, &logger_attr );
}

/*
 * Asynchronous Log Function (Ring Buffer Version)
 */
void LOG( const char* format, ... )
{
    char temp_buf[LOGGER_MAX_LINE];

    // 1. Format to local stack buffer first
    va_list args;
    va_start( args, format );
    int len = vsnprintf( temp_buf, LOGGER_MAX_LINE - 2, format, args );
    va_end( args );

    if ( len <= 0 )
        return;

    /* Automatically append \r\n if space permits */
    if (len < LOGGER_MAX_LINE - 2) {
        temp_buf[len++] = '\r';
        temp_buf[len++] = '\n';
    }

    // 2. Copy to Global Ring Buffer (Critical Section needed)
    uint32_t primask = __get_PRIMASK();
    __disable_irq(); // Disable interrupts to protect g_head and buffer write

    for ( int i = 0; i < len; i++ )
    {
        uint32_t next_head = ( g_head + 1 ) % LOGGER_BUFFER_SIZE;

        // Check for overflow (if full, we drop the CHAR, not the whole message
        // usually, or overwrite) Here we drop if full to preserve existing buffer
        if ( next_head != g_tail )
        {
            g_logBuffer[g_head] = temp_buf[i];
            g_head = next_head;
        }
        else
        {
            // Buffer full, stop writing this message
            break;
        }
    }

    if ( primask == 0 )
    {
        __enable_irq();
    }

    // 3. Notify Logger Task
    if ( loggerThread != NULL )
    {
        // osThreadFlagsSet is ISR-safe
        osThreadFlagsSet( loggerThread, 0x01 );
    }
}
