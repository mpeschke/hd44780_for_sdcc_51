/*
    Delay code copied from this repository: https://github.com/AguHDz/SDCC-8051/blob/master/Libs/delay.h
    on 2023-05-24. There was no information about licensing the code.
*/
#include <stdint.h>
#include "config.h"

#define CYCLES_PER_US(x) (x*1.0/(1000000.0/(XTAL_FREQ/MCU_CYCLE))+1)
// Define directive to make it easier to understand and
// automatically calculate the MCU cycles.
#define DELAY_10_TIMES_US(x) delay_x10_cycles(CYCLES_PER_US(x))

#define INST_CYCLE_NS (MCU_CYCLE*(1000000000/XTAL_FREQ))

#if INST_CYCLE_NS < 501
#define FAST_MCU
// A 10 us value seems to be adequate for all these delays,
// contrary to what's implemented in the original hd44780_pinbus.h.
// For fast MCUs those delays appear to be all equal to lcd_wait_2t(1)... ¯\_(ツ)_/¯
// Keeping the original names here for future reference though.
#define FN_DELAYT_W_EH   DELAY_10_TIMES_US(1)
#define FN_DELAYT_W_END  DELAY_10_TIMES_US(1)
#define FN_DELAYT_R_RS2E DELAY_10_TIMES_US(1)
#define FN_DELAYT_R_E2D  DELAY_10_TIMES_US(1)
#define FN_DELAYT_R_END  DELAY_10_TIMES_US(1)
#endif

extern void delay_x10_cycles(uint8_t x10cycles);
extern void delay_x100_cycles(uint8_t x100cycles);
extern void delay_ms(uint16_t ms);
extern void delay_5us();