// HITACHI HD44780 LCD driver for 8051 MCUs (SDCC ONLY)
// The hardware used for this code is actually a circuit module known as LCD 1602A, version 3.0.
//
// Author:                  apachiww@github.com
//
// Last modified time:      2021.09.26
//
// YOU MUST SETUP IOs IN THIS HEAD FILE AS 8051 DOES NOT SUPPORT INDIRECT ACCESS OF SFRs, they cannot be used as function arguments in SDCC
//
// Experimental auto delay calculation for MCUs using different crystals and operating under 1T, 6T and 12T modes, using MACROs
//
// Soft delay will be used when R/#W is not available to check the busy flag
//
// The common LCDs support 6 connection modes (Parallel M68 or i80 interface):
// 4bit-bus:                Use pins D5~D7 only, plus RS for register selection, R/#W for read/write selection and E for data latch (or #WR for write and #RD for read)
// 4bit-bus-no-read:        Same as 4bit-bus mode, but no read, R/#W is grounded (#WR connected to VCC)
// 8bit-bus:                Same as 4bit-bus except that it uses all pins D0~D7
// 8bit-bus-no-read:        Same as 8bit-bus mode with no read
// 8bit-bus-port:           Same as 8bit-bus mode, but uses the whole group IO e.g. P2
// 8bit-bus-port-no-read:   No read

// Usage:
// 1. Define your connection mode, available options are LCD_BUS_4BIT, LCD_BUS_8BIT or LCD_BUS_8P
// 2. Define LCD_NO_READ if you do not use the read function, and soft delay will be used instead of waiting for the busy flag
// 3. Change the definitions of IOs, XTAL_FREQ and MCU_CYCLE accordingly, this will be used in the calculation of delays later

// NOTE:
// 1. If the instruction cycle is shorter than 500nS (i.e. you use a crystal of 33.1176Mhz and operate under 1T mode, it would be 30.195nS), the
// MCU is considered a FAST MCU, and the IO operation would be too fast to control the LCD, so delay-in-trasfer will be enabled automatically

#include <mcs51/8051.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

/*--------------------------Begin of user defined options----------------------------*/

/*---------Set your 8051 XTAL Freq (default 12T mode), required for soft delay-------*/

// Assume XTAL_FREQ = 11.0592Mhz and using 12T mode, the result would be 1.085uS per instruction

#define XTAL_FREQ 22118400
#define MCU_CYCLE 12

/*------------------------------Select your IO Mode----------------------------------*/

// #define IO_MODE_I80
#define IO_MODE_M68

/*---------------------------Select your connect mode--------------------------------*/

#define LCD_BUS_4BIT
// #define LCD_BUS_8BIT
// #define LCD_BUS_8P

// #define LCD_NO_READ

/*----------Uncomment the following options to enable light adjust for VFDs----------*/

// #define DISP_TYPE_NORITAKE_CU20045
// #define DISP_TYPE_PT6314

/*----------IO definitions. Change the definitions of IOs below accordingly----------*/

#define IO_RS       P1_6    // The RS pin 
#define IO_RW_RD    P1_5    // RW pin in m68 mode or RD pin in i80 mode. Notice that usually the RW pin in m68 mode is multiplexed with WR in i80 mode
#define IO_E_WR     P1_4    // Enable pin in m68 mode or WR pin in i80 mode
#ifdef  LCD_NO_READ
#undef  IO_RW_RD
#endif

#ifdef  LCD_BUS_4BIT
#define IO_D4   P1_0
#define IO_D5   P1_1
#define IO_D6   P1_2
#define IO_D7   P1_3
#endif

#ifdef  LCD_BUS_8BIT
#define IO_D0   P2_0
#define IO_D1   P2_1
#define IO_D2   P2_2
#define IO_D3   P2_3
#define IO_D4   P2_4
#define IO_D5   P2_5
#define IO_D6   P2_6
#define IO_D7   P2_7
#endif

#ifdef  LCD_BUS_8P
#define IO_D    P2
#endif

/*--------------------------End of user defined options----------------------------*/

// Calculate delay cycles

#define INST_CYCLE_NS (MCU_CYCLE*(1000000000/XTAL_FREQ))

#if INST_CYCLE_NS > 500
#define FN_DELAY_PWRON          lcd_wait_512t((15000000/INST_CYCLE_NS)/512 + 1)     // Power on delay
#define FN_DELAY_CLR            lcd_wait_512t((1520000/INST_CYCLE_NS)/512 + 1)      // Reset or clear delay
#define FN_DELAY_CMD            lcd_wait_2t((37000/INST_CYCLE_NS)/2 + 1)            // Command execution delay
#define FN_DELAY_INIT_PHASE1    lcd_wait_512t((4100000/INST_CYCLE_NS)/512 + 1)      // Delay during the 1st phase of init
#define FN_DELAY_INIT_PHASE2    lcd_wait_2t((100000/INST_CYCLE_NS)/2 + 1)           // Delay during the 2nd phase of init

#else
#define FAST_MCU                                                                    // Enable extra delay when using fast MCU
#define FN_DELAY_PWRON          lcd_wait_65kt((15000000/INST_CYCLE_NS)/131072 + 1)
#define FN_DELAY_CLR            lcd_wait_512t((1520000/INST_CYCLE_NS)/512 + 1)
#define FN_DELAY_CMD            lcd_wait_512t((37000/INST_CYCLE_NS)/512 + 1)
#define FN_DELAY_INIT_PHASE1    lcd_wait_65kt((4100000/INST_CYCLE_NS)/131072 + 1)  
#define FN_DELAY_INIT_PHASE2    lcd_wait_512t((100000/INST_CYCLE_NS)/512 + 1)

#define FN_DELAYT_W_EH          lcd_wait_2t((450/INST_CYCLE_NS)/2 + 1)              // Enable High status delay
#define FN_DELAYT_W_END         lcd_wait_2t((400/INST_CYCLE_NS)/2 + 1)              // Delay after an operation finishes

#ifndef LCD_NO_READ
#define FN_DELAYT_R_RS2E        lcd_wait_2t((60/INST_CYCLE_NS)/2 + 1)               // Delay before E goes high while reading 
#define FN_DELAYT_R_E2D         lcd_wait_2t((360/INST_CYCLE_NS)/2 + 1)              // Delay before data is ready
#define FN_DELAYT_R_END         lcd_wait_2t((400/INST_CYCLE_NS)/2 + 1)              // Delay after an operation finishes
#endif
#endif

// If read function available, check busy flag rather than soft delay

#ifndef LCD_NO_READ
#define DELAY_CMD lcd_wait()
#define DELAY_CLR lcd_wait()
#else
#define DELAY_CMD FN_DELAY_CMD
#define DELAY_CLR FN_DELAY_CLR
#endif

// Command definitions

#define CMD_CLEAR               0x01

#define CMD_HOME                0x02

#define CMD_ENTRY               0x04
#define CMD_ENTRY_SHIFT         0x01
#define CMD_ENTRY_CURSOR        0x00
#define CMD_ENTRY_INC           0x02
#define CMD_ENTRY_DEC           0x00

#define CMD_SET_DISP            0x08
#define CMD_SET_DISP_ON         0x04
#define CMD_SET_DISP_OFF        0x00
#define CMD_SET_CUR_ON          0X02
#define CMD_SET_CUR_OFF         0x00
#define CMD_SET_BLINK_ON        0x01
#define CMD_SET_BLINK_OFF       0x00

#define CMD_MOVE                0x10
#define CMD_MOVE_RIGHT          0x04
#define CMD_MOVE_LEFT           0x00
#define CMD_MOVE_DISP           0x08
#define CMD_MOVE_CURSOR         0x00

#define CMD_INIT                0x20
#define CMD_INIT_8_BIT          0x10
#define CMD_INIT_4_BIT          0x00
#define CMD_INIT_2_LINE         0x08
#define CMD_INIT_1_LINE         0x00
#define CMD_INIT_8_FONT         0x00     
#define CMD_INIT_10_FONT        0x04

#define CMD_SET_ACG             0x40

#define CMD_SET_ADD             0x80


// Basic level IO functions

void write_cmd(uint8_t);
void write_4bit(uint8_t);
void write_data(uint8_t);

#ifndef LCD_NO_READ
uint8_t read_bf_addr();
uint8_t read_data();
void lcd_wait();                // Wait for BF before write data
#endif
void lcd_wait_2t(uint8_t);      // Soft delay, approximately 2*t instruction cycles
void lcd_wait_512t(uint8_t);    // Delay approximately 2*256*t cycles
void lcd_wait_65kt(uint8_t);    // Delay 2*65536*t


// Medium level functions, use these functions if you know the working process of the LCDs/VFDs well

void lcd_clear();                                       // Display clear
void lcd_home();                                        // Cursor home and display go back to the original position

void lcd_init(uint8_t);                                 // Input the number of columns and rows 

void lcd_set_entry(uint8_t);                            // Set your entry mode
void lcd_set_disp(uint8_t);                             // Set your display mode, like display on/off, cursor on/off and cursor blink

void lcd_mov(uint8_t);                                  // Move the cursor or the whole display by one position

void lcd_cpy_ddram(const char *, uint8_t);              // Use this function to write muitiple bytes to DDRAM, e.g. a string
void lcd_put_cur_addr(uint8_t);                         // Set the cursor location, direct address in DDRAM (00H~4FH under 1 line mode, 00H~27H 40H~67H under 2 line mode) 

void lcd_cpy_cgram(const uint8_t *, uint8_t);           // Use this function to write muitiple bytes to CGRAM
void lcd_put_cg_addr(uint8_t);                          // Set direct address in CGRAM (00H~3FH)

#ifndef LCD_NO_READ         
uint8_t lcd_get_cur_addr();                             // Get the address of cursor 
#endif

#ifdef DISP_TYPE_NORITAKE_CU20045
void vfd_set_light_cu20045(uint8_t);                    // Set the lightness of the VFD, from 0 to 3 
#endif

#ifdef DISP_TYPE_PT6314
void vfd_set_light_pt6314(uint8_t);                     // Set the lightness of the VFD, from 0 to 3 
#endif

// High level functions, recommended

void disp_start(uint8_t, uint8_t);                      // Use this function to initialize the LCD if the power supply does not meet the requirements of power-on reset
void disp_clear();                                      // Clear LCD. Identical to lcd_clear()
void disp_home();                                       // Cursor home. Identical to lcd_home()

void disp_on();                                         // Enable display
void disp_off();                                        // Display off
void disp_cur_on();                                     // Cursor blink on
void disp_cur_off();                                    // Cursor blink off

void disp_put_cur(uint8_t, uint8_t);                    // Set cursor position
#ifndef LCD_NO_READ
void disp_get_cur(uint8_t *, uint8_t *);                // Get cursor position
#endif

void disp_shift(uint8_t);                               // Shift the display by one position, pass 1 to shift right and 0 left 
void disp_curmov(uint8_t);                              // Move the cursor by one position, pass 1 to move right and 0 left

void disp_print(char);                                  // Print a single character
void disp_println(const char *, uint8_t);               // Print line

int disp_printf(const char *, ...);                     // Formattable print function, to support usage similar to printf()