#include "cmsis_os2.h"
#include "macro.h"

/* Timer handles */
osTimerId_t xOneShotTimer;
osTimerId_t xAutoReloadTimer1;
osTimerId_t xAutoReloadTimer2;
osTimerId_t xLongNameTimer;

/* Global counters to replace TimerID storage */
static uint32_t ulOneShotCount = 0;
static uint32_t ulAutoReload1Count = 0;
static uint32_t ulAutoReload2Count = 0;
static uint32_t ulLongNameCount = 0;

/* Timer callback function */
void timer_cb(void *argument)
{
    uint32_t *pulCount = (uint32_t *)argument;
    if (pulCount != NULL) {
        (*pulCount)++;
    }
}

void create_timer_test(void)
{
    /* 1. One-Shot Timer
       Name: "OneShot"
       Period: 3333 ms
       Mode: osTimerOnce (One-Shot)
    */
    const osTimerAttr_t oneShotAttr = {
        .name = "OneShot"
    };
    xOneShotTimer = osTimerNew(timer_cb, osTimerOnce, &ulOneShotCount, &oneShotAttr);

    /* 2. Auto-Reload Timer 1 (Short Period)
       Name: "AutoReload_1"
       Period: 200 ms
       Mode: osTimerPeriodic (Auto-Reload)
    */
    const osTimerAttr_t autoReload1Attr = {
        .name = "AutoReload_1"
    };
    xAutoReloadTimer1 = osTimerNew(timer_cb, osTimerPeriodic, &ulAutoReload1Count, &autoReload1Attr);

    /* 3. Auto-Reload Timer 2 (Long Period)
       Name: "AutoReload_2"
       Period: 5000 ms
       Mode: osTimerPeriodic (Auto-Reload)
    */
    const osTimerAttr_t autoReload2Attr = {
        .name = "AutoReload_2"
    };
    xAutoReloadTimer2 = osTimerNew(timer_cb, osTimerPeriodic, &ulAutoReload2Count, &autoReload2Attr);

    /* 4. Timer with a long name
       Name: "LongNameTest"
       Period: 1000 ms
       Mode: osTimerPeriodic (Auto-Reload)
    */
    const osTimerAttr_t longNameAttr = {
        .name = "LongNameTest"
    };
    xLongNameTimer = osTimerNew(timer_cb, osTimerPeriodic, &ulLongNameCount, &longNameAttr);

    /* Start the timers. 
       Note: cmsis_os2 uses ticks. Assuming standard mapping or just using raw values 
       similar to previous pdMS_TO_TICKS if we assume 1 tick = 1 ms for simplicity 
       or use the calculation logic if available.
       Since we don't have pdMS_TO_TICKS, we will assume the previous values 
       were essentially milliseconds and the system tick is 1ms or handle it generically.
       
       We'll use osKernelGetTickFreq() to calculate ticks from ms if we want to be precise,
       but for this test, passing the raw number (assuming 1ms tick) is standard for many setups.
       If the previous code used pdMS_TO_TICKS(3333), and configTICK_RATE_HZ is 1000, it's 3333.
    */
    
    // Helper to convert ms to ticks roughly
    uint32_t freq = osKernelGetTickFreq(); // usually 1000
    
    CUSTOM_ASSERT(osTimerStart(xOneShotTimer, (3333 * freq) / 1000) == osOK);
    CUSTOM_ASSERT(osTimerStart(xAutoReloadTimer1, (200 * freq) / 1000) == osOK);
    CUSTOM_ASSERT(osTimerStart(xAutoReloadTimer2, (5000 * freq) / 1000) == osOK);
    CUSTOM_ASSERT(osTimerStart(xLongNameTimer, (1000 * freq) / 1000) == osOK);
}
