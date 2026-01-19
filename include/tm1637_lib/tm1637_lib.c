#include "tm1637_lib.h"

/******************************************************************************/
/*                             Internal definitions                           */
/******************************************************************************/
#define constrain(amt, low, high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define TM1637_DATA_CMD_SETTING         0x40
#define TM1637_DISPLAY_CTRL_CMD_SETTING 0x80
#define TM1637_ADDRESS_CMD_SETTING      0xC0

#define DATA_RW_MODE_WRITE    0x00
#define DATA_RW_MODE_READ     0x02

#define TM1637_ADDR_ADD_MODE_AUTO_INC 0x00
#define TM1637_ADDR_ADD_MODE_FIXED    0x04

#define TM1637_DISPLAY_SW_OFF 0x00
#define TM1637_DISPLAY_SW_ON  0x08

/******************************************************************************/
/*                        Static function definitions                         */
/******************************************************************************/
static void clkSet_high(tm1637Ctxt_t *me)
{
    *(me->clkPORT) |= (1 << me->clkBIT);
}

static void clkSet_low(tm1637Ctxt_t *me)
{
    *(me->clkPORT) &= ~(1 << me->clkBIT);
}

static void clkPulse(tm1637Ctxt_t *me)
{
    clkSet_high(me);
    TM16_DELAY_US(5);
    clkSet_low(me);
    TM16_DELAY_US(5);
}

static void dioSet_high(tm1637Ctxt_t *me)
{
    *(me->dioPORT) |= (1 << me->dioBIT);
}

static void dioSet_low(tm1637Ctxt_t *me)
{
    *(me->dioPORT) &= ~(1 << me->dioBIT);
}

static void dioSet_input(tm1637Ctxt_t *me)
{
    *(me->dioDDR) &= ~(1 << me->dioBIT);
}

static void dioSet_output(tm1637Ctxt_t *me)
{
    *(me->dioDDR) |= (1 << me->dioBIT);
}

static uint8_t dioRead(tm1637Ctxt_t *me)
{
    return (*(me->dioPIN) & (1 << me->dioBIT)) ? 1 : 0;
}

static void tm1617_start(tm1637Ctxt_t *me)
{
    dioSet_output(me);

    dioSet_high(me);
    clkSet_high(me);
    TM16_DELAY_US(2);
    dioSet_low(me);
    TM16_DELAY_US(2);
    clkSet_low(me);
}

static void tm1617_stop(tm1637Ctxt_t *me)
{
    dioSet_output(me);

    clkSet_low(me);
    dioSet_low(me);
    TM16_DELAY_US(2);
    clkSet_high(me);
    TM16_DELAY_US(2);
    dioSet_high(me);
    TM16_DELAY_US(2);
}

static bool tm1617_writeByte(tm1637Ctxt_t *me, uint8_t data)
{
    bool ret = true;

    dioSet_output(me);
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x01) {
            dioSet_high(me);
        } else {
            dioSet_low(me);
        }
        TM16_DELAY_US(5);
        clkPulse(me);
        data >>= 1;
    }

    // Wait for ACK
    dioSet_input(me);
    TM16_DELAY_US(5);

    if (dioRead(me)) {
        ret = false; // No ACK received
    }

    clkPulse(me);
    TM16_DELAY_US(5);

    if (!dioRead(me)) {
        ret = false; // No ACK received
    }

    dioSet_output(me);
    return ret;
}

static uint8_t tm1637_encodeChar(char c)
{
    switch (c) {
        // --- digits ---
        case '0': return 0x3F;
        case '1': return 0x06;
        case '2': return 0x5B;
        case '3': return 0x4F;
        case '4': return 0x66;
        case '5': return 0x6D;
        case '6': return 0x7D;
        case '7': return 0x07;
        case '8': return 0x7F;
        case '9': return 0x6F;

        // --- hexadecimal letters with native 7-segment representation ---
        case 'A': case 'a': return 0x77;
        case 'B': case 'b': return 0x7C;
        case 'C': case 'c': return 0x39;
        case 'D': case 'd': return 0x5E;
        case 'E': case 'e': return 0x79;
        case 'F': case 'f': return 0x71;

        // --- supported symbols (no decimal point) ---
        case '-': return 0x40; // minus (segment G)
        case '_': return 0x08; // underscore (segment D)

        // --- other characters ---
        case 'H': case 'h': return 0x76;
        case 'I': case 'i': return 0x06;
        case 'J': case 'j': return 0x1E;
        case 'L': case 'l': return 0x38;
        case 'O': case 'o': return 0x5C;
        case 'P': case 'p': return 0x73;
        case 'S': case 's': return 0x6D;
        case 'T': case 't': return 0x78;
        case 'U': case 'u': return 0x3E;
        case 'V': case 'v': return 0x1C;
        case 'Y': case 'y': return 0x6E;

        // --- unsupported characters ---
        default:  return 0x00; // turn off the digit completely
    }
}

/******************************************************************************/
/*                        Public function definitions                         */
/******************************************************************************/
void tm1637_initHw(tm1637Ctxt_t *me)
{
    // Set CLK and DIO as outputs
    *(me->clkDDR) |= (1 << me->clkBIT);
    *(me->dioDDR) |= (1 << me->dioBIT);

    // Set CLK and DIO low
    *(me->clkPORT) &= ~(1 << me->clkBIT);
    *(me->dioPORT) &= ~(1 << me->dioBIT);
}

////////////////////////////////////////////////////////////////////////////////

void tm1637_setBrightness(tm1637Ctxt_t *me, uint8_t brightness)
{
    me->brightness = constrain(brightness, TM1637_BRIGHTNESS_MIN, TM1637_BRIGHTNESS_MAX);

    tm1617_start(me);
    tm1617_writeByte(me, TM1637_DISPLAY_CTRL_CMD_SETTING | 
                            TM1637_DISPLAY_SW_ON |
                            me->brightness);
    tm1617_stop(me);
}

////////////////////////////////////////////////////////////////////////////////

void tm1637_dispSwitch(tm1637Ctxt_t *me, uint8_t sw)
{
    tm1617_start(me);
    tm1617_writeByte(me, TM1637_DISPLAY_CTRL_CMD_SETTING |
                            (sw ? TM1637_DISPLAY_SW_ON : TM1637_DISPLAY_SW_OFF) |
                            me->brightness);
    tm1617_stop(me);
}

////////////////////////////////////////////////////////////////////////////////

void tm1637_dispMode(tm1637Ctxt_t *me, tm1637_dispMode_t mode)
{
    tm1617_start(me);
    tm1617_writeByte(me, TM1637_DATA_CMD_SETTING |
                            DATA_RW_MODE_WRITE |
                            mode);
    tm1617_stop(me);
}

////////////////////////////////////////////////////////////////////////////////

void tm1637_print(tm1637Ctxt_t *me, const char *str)
{
    char paddedStr[TM1637_REGS_COUNT] = {' '};

    memcpy((paddedStr + (TM1637_REGS_COUNT - strlen(str))), str, strlen(str));

    tm1617_start(me);
    tm1617_writeByte(me, TM1637_ADDRESS_CMD_SETTING |
                        DATA_RW_MODE_WRITE |
                        TM1637_ADDR_ADD_MODE_AUTO_INC |
                        TM1637_REG_ADDR_MIN);

    for (uint8_t i = (TM1637_REGS_COUNT - me->digits); i < TM1637_REGS_COUNT; i++) {
        uint8_t encodedChar = tm1637_encodeChar(paddedStr[i]);
        tm1617_writeByte(me, encodedChar);
    }

    tm1617_stop(me);
}

////////////////////////////////////////////////////////////////////////////////

void tm1637_setDispRegAddr(tm1637Ctxt_t *me, uint8_t addr)
{
    uint8_t addrConstrained = constrain(addr, TM1637_REG_ADDR_MIN, TM1637_REG_ADDR_MAX);

    tm1617_start(me);
    tm1617_writeByte(me, TM1637_ADDRESS_CMD_SETTING | addrConstrained);
    tm1617_stop(me);
}
