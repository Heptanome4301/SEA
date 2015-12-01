#ifndef HW_H
#define HW_H

#include "config.h"

#if SERIAL_PRINT
#include "uart.h"
#endif

#include <stdint.h>

/****************** Printing  ***********/
#if SERIAL_PRINT
#define log_str(msg) uart_send_str(">>>> "); uart_send_str(msg);
#define log_int(msg) uart_send_int(msg);
#define log_cr() uart_send_str("\n");
#else
#define log_str(msg)
#define log_int(msg)
#define log_cr()
#endif

/*************  Memory mapping ***********/
#define HEAP_START 0x50000
#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE         (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define PWM_BASE          (BCM2708_PERI_BASE + 0x20C000) /* PWM controller */
#define CLOCK_BASE        (BCM2708_PERI_BASE + 0x101000) /* Address */

/*********** Processor modes *************/
#define IRQ_MODE 0x12
#define SVC_MODE 0x13
#define SYS_MODE 0x1F

/************** Clock module *************/
#define CS      0x20003000
#define CLO     0x20003004
#define CHI     0x20003008
#define C0      0x2000300C
#define C1      0x20003010
#define C2      0x20003014
#define C3      0x20003018

#define SYS_TIMER_CLOCK 250000000 /* We use the system timer, running at 250MHz */
#define SYS_TIMER_CLOCK_div_1000 250000
#define DEFAULT_TIMER_INTERVAL 2500000 /* 10 ms */

#define ENABLE_TIMER_IRQ() Set32(CS,2)
#define DISABLE_TIMER_IRQ() Set32(CS,~2);


/******************* GPIO ***************/
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028

/****************** PWM *****************/
#define BCM2835_PWM_CONTROL 0
#define BCM2835_PWM_STATUS  1
#define BCM2835_PWM0_RANGE  4
#define BCM2835_PWM0_DATA   5
#define BCM2835_PWM_FIFO    6
#define BCM2835_PWM1_RANGE  8
#define BCM2835_PWM1_DATA   9

#define BCM2835_PWMCLK_CNTL   40
#define BCM2835_PWMCLK_DIV    41

#define BCM2835_PWM1_MS_MODE    0x8000  /*  Run in MS mode                   */
#define BCM2835_PWM1_USEFIFO    0x2000  /*  Data from FIFO                   */
#define BCM2835_PWM1_REVPOLAR   0x1000  /* Reverse polarity                  */
#define BCM2835_PWM1_OFFSTATE   0x0800  /* Ouput Off state                   */
#define BCM2835_PWM1_REPEATFF   0x0400  /* Repeat last value if FIFO empty   */
#define BCM2835_PWM1_SERIAL     0x0200  /* Run in serial mode                */
#define BCM2835_PWM1_ENABLE     0x0100  /* Channel Enable                    */

#define BCM2835_PWM0_MS_MODE    0x0080  /* Run in MS mode                    */
#define BCM2835_PWM0_USEFIFO    0x0020  /* Data from FIFO                    */
#define BCM2835_PWM0_REVPOLAR   0x0010  /* Reverse polarity                  */
#define BCM2835_PWM0_OFFSTATE   0x0008  /* Ouput Off state                   */
#define BCM2835_PWM0_REPEATFF   0x0004  /* Repeat last value if FIFO empty   */
#define BCM2835_PWM0_SERIAL     0x0002  /* Run in serial mode                */
#define BCM2835_PWM0_ENABLE     0x0001  /* Channel Enable                    */

/*************** FIFO ERROR CODES **************/
#define BCM2835_BERR  0x100
#define BCM2835_GAPO4 0x80
#define BCM2835_GAPO3 0x40
#define BCM2835_GAPO2 0x20
#define BCM2835_GAPO1 0x10
#define BCM2835_RERR1 0x8
#define BCM2835_WERR1 0x4
#define BCM2835_EMPT1 0x2
#define BCM2835_FULL1 0x1
#define ERRORMASK (BCM2835_GAPO2 | BCM2835_GAPO1 | BCM2835_RERR1 | BCM2835_WERR1)

#define PM_PASSWORD 0x5A000000

/**************** Watchdog ***********************/
#define PM_RSTC 0x2010001c
#define PM_WDOG 0x20100024
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020

/*************** Functions declaration *****************/
uint64_t get_date_ms();
void set_date_ms(uint64_t date_ms);
void set_next_tick(uint32_t time_ms);
void set_next_tick_default();

void enable_timer_irq();
void disable_timer_irq();
void timer_init();

void led_on(void);
void led_off(void);
void led_switch(void);
void led_blink(void);

void hw_init();
void terminate_kernel();

#endif
