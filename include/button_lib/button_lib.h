#ifndef _BUTTON_LIB_H_
#define _BUTTON_LIB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pin;
    uint8_t bit;

    uint8_t activeState;
    uint8_t pullUpEn;
} buttonCtxt_t;

#define BUTTON_DECLARE_CTXT(name, buttPort, buttBit, actState, isPullUp) \
    buttonCtxt_t name = { \
        /* DDR register is PORT - 1 */ \
        /* PIN register is PORT - 2 */ \
        .ddr = &(buttPort), \
        .port = &(buttPort), \
        .pin = &(buttPort) - 2, \
        .bit = (buttBit), \
        .activeState = (actState), \
        .pullUpEn = (isPullUp), \
    };

#define TM16_DELAY_US(us) _delay_us(us) // Placeholder for delay function

/**
 * @fn button_initHw
 * @param me - Pointer to the button context structure.
 * @brief Initialize the button hardware (GPIO pins).
 */
void button_initHw(buttonCtxt_t *me);

/**
 * @fn button_isPressed
 * @param me - Pointer to the button context structure.
 * @return 1 if the button is pressed, 0 otherwise.
 * @brief Check if the button is currently pressed.
 */
uint8_t button_isPressed(buttonCtxt_t *me);

/* _BUTTON_LIB_H_ */
#endif
