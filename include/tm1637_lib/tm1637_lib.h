#ifndef _TM1637_LIB_H_
#define _TM1637_LIB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#define TM1637_BRIGHTNESS_MIN 0x00
#define TM1637_BRIGHTNESS_MAX 0x07

#define TM1637_REG_ADDR_MIN 0x00
#define TM1637_REG_ADDR_MAX 0x05
#define TM1637_REGS_COUNT   (TM1637_REG_ADDR_MAX + 1)

typedef enum {
    tm1637_dispMode_normal = 0x00,
    tm1637_dispMode_test = 0x08
} tm1637_dispMode_t;

typedef struct {
    volatile uint8_t *clkDDR;
    volatile uint8_t *clkPORT;
    uint8_t clkBIT;

    volatile uint8_t *dioDDR;
    volatile uint8_t *dioPORT;
    volatile uint8_t *dioPIN;
    uint8_t dioBIT;

    uint8_t brightness;
    tm1637_dispMode_t dispMode;
    const uint8_t digits;
} tm1637Ctxt_t;

#define TM16_DECLARE_CTXT(name, clkPort, clkBit, dioPort, dioBit, digitNum) \
    tm1637Ctxt_t name = { \
        /* DDR register is PORT - 1 */ \
        /* PIN register is PORT - 2 */ \
        .clkPORT = &(clkPort), \
        .clkDDR = &(clkPort) - 1, \
        .clkBIT = (clkBit), \
        .dioPORT = &(dioPort), \
        .dioDDR = &(dioPort) - 1, \
        .dioPIN = &(dioPort) - 2, \
        .dioBIT = (dioBit), \
        .brightness = TM1637_BRIGHTNESS_MAX, \
        .dispMode = tm1637_dispMode_normal, \
        .digits = (digitNum) \
    };

#define TM16_DELAY_US(us) _delay_us(us) // Placeholder for delay function

/**
 * @fn tm1637_initHw
 * @param me - Pointer to the TM1637 context structure.
 * @brief Initialize the TM1637 hardware (GPIO pins).
 */
void tm1637_initHw(tm1637Ctxt_t *me);

/**
 * @fn tm1637_setBrightness
 * @param me - Pointer to the TM1637 context structure.
 * @param brightness - Brightness level (0-7).
 * @brief Set the brightness of the TM1637 display.
 */
void tm1637_setBrightness(tm1637Ctxt_t *me, uint8_t brightness);

/**
 * @fn tm1637_dispMode
 * @param me - Pointer to the TM1637 context structure.
 * @param mode - Display mode (normal or test).
 * @brief Set the display mode of the TM1637.
 */
void tm1637_dispMode(tm1637Ctxt_t *me, tm1637_dispMode_t mode);

/**
 * @fn tm1637_dispSwitch
 * @param me - Pointer to the TM1637 context structure.
 * @param sw - Switch display on (1) or off (0).
 * @brief Switch the TM1637 display on or off.
 */
void tm1637_dispSwitch(tm1637Ctxt_t *me, uint8_t sw);

/**
 * @fn tm1637_setDispRegAddr
 * @param me - Pointer to the TM1637 context structure.
 * @param addr - Address command to set the display register address.
 * @brief Set the display register address for the TM1637.
 */
// void tm1637_setDispRegAddr(tm1637Ctxt_t *me, tm1637_addressCmd_t addr);

/**
 * @fn tm1637_print
 * @param me - Pointer to the TM1637 context structure.
 * @param str - String to display (up to 4 characters).
 * @brief Write a string to the TM1637 display.
 */
void tm1637_print(tm1637Ctxt_t *me, const char *str);

void tm1637_test(tm1637Ctxt_t *me);

/* _TM1637_LIB_H_ */
#endif
