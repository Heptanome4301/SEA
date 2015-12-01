#include <stdint.h>

#include "hw.h"
#include "asm_tools.h"

/***************************
 ******** Utilities ********
 ***************************/
uint64_t
divide(uint64_t x, uint64_t y) {
    int quotient = 0;
    while (x >= y) {
        x  -=  y;
        quotient++;
    }
    return quotient;
}

/* ***************************
 * ********** Timer **********
 * ***************************/
uint64_t
get_date_ms()
{
#if RPI
    uint32_t date_lowbits = Get32(CLO);
    uint64_t date_highbits = (uint64_t) Get32(CHI);
    uint64_t date = divide(((date_highbits << 32) | date_lowbits), SYS_TIMER_CLOCK_div_1000);
#else
    uint64_t date = ((uint64_t) 0x43 << 32) | 0x42;
#endif

    return date;
}

void
set_date_ms(uint64_t date_ms)
{
  uint64_t date = date_ms * SYS_TIMER_CLOCK_div_1000;
  uint32_t date_lowbits = date & (0x0000FFFF);
  uint32_t date_highbits = date >> 32;
  Set32(CLO, date_lowbits);
  Set32(CHI, date_highbits);
}

void
set_next_tick(uint32_t time_ms)
{
    uint32_t date_lowbits = Get32(CLO);
    date_lowbits += (uint32_t) (time_ms * SYS_TIMER_CLOCK_div_1000) ;
    Set32(C1, date_lowbits);
}

void
set_next_tick_default()
{
    uint32_t date_lowbits = Get32(CLO);
    date_lowbits += DEFAULT_TIMER_INTERVAL;
    Set32(C1, date_lowbits);
}

/* Use *system timer* peripheral -> compare module CM1 */
void
timer_init()
{
    /* 10 ms seems good */
    set_next_tick_default();

    /* Enable timer irq */
    ENABLE_TIMER_IRQ();

    /* Enable interrupt *line* */
    Set32(0x2000B210, 0x00000002);
}

/* **************************
 * ********** LEDs **********
 * **************************/
static unsigned int led_state = 0U;

void
led_on(void)
{
    unsigned int *gpclr0_ptr = ((unsigned int*)GPCLR0);
    *gpclr0_ptr = *gpclr0_ptr | 0x10000U;
    led_state = 1;
}

void
led_off(void)
{
    unsigned int *gpclr0_ptr = ((unsigned int*)GPSET0);
    *gpclr0_ptr = *gpclr0_ptr | 0x10000U;
    led_state = 0;
}

void
led_switch(void)
{
    if ((led_state & 1U) == 0U)
    {
        led_off();
    }
    else
    {
        led_on();
    }
}

unsigned int
led_status(void)
{
    return led_state;
}

void
led_init(void)
{
    unsigned int *gpfsel1_ptr = ((unsigned int*)GPFSEL1);
    *gpfsel1_ptr = *gpfsel1_ptr | 0x40000U;
    led_off();
}

void
led_blink(void)
{
    int i =0;
    int waiting = 0;
    for (i = 0; i < 10; i++) {
        led_on();
        for (waiting = 0; waiting < 1000; waiting++);
        led_off();
        for (waiting = 0; waiting < 1000; waiting++);                
    }
}

/* ***********************************
 * ********** init_hw() **************
 * ***********************************/
void
hw_init()
{
    /* Init uart */
    uart_init();

    /* Init LED */
    led_init();
}

void
terminate_kernel()
{
    log_str("Exit kernel\n");
}
