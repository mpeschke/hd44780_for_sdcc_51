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

extern void delay_x10_cycles(uint8_t x10cycles);
extern void delay_x100_cycles(uint8_t x100cycles);
extern void delay_ms(uint16_t ms);
extern void delay_5us();