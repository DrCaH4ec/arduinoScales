#include "button_lib.h"

/******************************************************************************/
/*                        Static function definitions                         */
/******************************************************************************/

/******************************************************************************/
/*                        Public function definitions                         */
/******************************************************************************/
void button_initHw(buttonCtxt_t *me)
{
    // Set button pin as input
    *(me->ddr) &= ~(1 << me->bit);

    if (me->pullUpEn) {
        // Enable pull-up resistor
        *(me->port) |= (1 << me->bit);
    } else {
        // Disable pull-up resistor
        *(me->port) &= ~(1 << me->bit);
    }
}

////////////////////////////////////////////////////////////////////////////////

uint8_t button_isPressed(buttonCtxt_t *me)
{
    uint8_t pinState = (*(me->pin) & (1 << me->bit)) ? 1 : 0;
    return (pinState == me->activeState) ? 1 : 0;
}
