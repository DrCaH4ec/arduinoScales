#include "xh17_lib.h"

/******************************************************************************/
/*                        Static function definitions                         */
/******************************************************************************/

static void pdSckSet_high(xh17Ctxt_t *me)
{
    *me->pdSckPORT |= (1 << me->pdSckBIT);
    XH17_DELAY_US(1);
}

static void pdSckSet_low(xh17Ctxt_t *me)
{
    *me->pdSckPORT &= ~(1 << me->pdSckBIT);
    XH17_DELAY_US(1);
}

static uint8_t dOutRead(xh17Ctxt_t *me)
{
    return (*me->dOutPIN & (1 << me->dOutBIT)) ? 1 : 0;
}

static inline uint32_t u32AbsDiff(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

/******************************************************************************/
/*                        Public function definitions                         */
/******************************************************************************/
void xh17_setInputSelect(xh17Ctxt_t *me, xh17_inputSelect_t inputSelect)
{
    me->inputSelect = inputSelect;

    pdSckSet_low(me);
    (void)xh17_readRaw(me); // Dummy read to apply new input select
}

void xh17_initHw(xh17Ctxt_t *me)
{
    *me->dOutDDR  &= ~(1 << me->dOutBIT); // Set DOUT as input
    *me->dOutPORT |= (1 << me->dOutBIT);  // Enable pull-up resistor on DOUT
    *me->pdSckDDR |= (1 << me->pdSckBIT); // Set PD_SCK as output
    pdSckSet_low(me);

    xh17_setInputSelect(me, me->inputSelect); // Apply initial input select
}

////////////////////////////////////////////////////////////////////////////////

bool xh17_isReady(xh17Ctxt_t *me)
{
    return !dOutRead(me);
}

////////////////////////////////////////////////////////////////////////////////

void xh17_waitUntilReady(xh17Ctxt_t *me)
{
    while (!xh17_isReady(me));
}

////////////////////////////////////////////////////////////////////////////////

int32_t xh17_readRaw(xh17Ctxt_t *me)
{
    int32_t count = 0;
    uint8_t i;
    
    xh17_waitUntilReady(me);

    for (i = 0; i < 24; i++) {
        pdSckSet_high(me);
        count = count << 1;
        pdSckSet_low(me);
        count |= dOutRead(me) ? 0x01 : 0x00;
    }

    if (me->inputSelect == xh17_inputSelect_B_32) {
        pdSckSet_high(me);
        pdSckSet_low(me);
    }

    if (me->inputSelect == xh17_inputSelect_A_64) {
        pdSckSet_high(me);
        pdSckSet_low(me);
    }

    count ^= 0x800000; // Set the sign bit

    return count;
}

////////////////////////////////////////////////////////////////////////////////

int32_t xh17_readFiltered(xh17Ctxt_t *me)
{
    int32_t x = xh17_readRaw(me);

    if (!me->filtInited) {
        me->count = x;
        me->countOut = x;
        me->filtInited = 1;
    } else {
        // ---- ADAPTIVE ALPHA ----
        int32_t d = u32AbsDiff(x, me->count);
        uint8_t alpha_q8;

        if (d <= me->dLow) {
            alpha_q8 = me->alphaMin_q8;
        } else if (d >= me->dHigh) {
            alpha_q8 = me->alphaMax_q8;
        } else {
            uint32_t num = (d - me->dLow) * (uint32_t)(me->alphaMax_q8 - me->alphaMin_q8);
            uint32_t den = (me->dHigh - me->dLow);
            alpha_q8 = (uint8_t)(me->alphaMin_q8 + (num / den));
        }

        // ---- EMA UPDATE (internal) ----
        int32_t diff = (int32_t)x - (int32_t)me->count;
        me->count = (int32_t)me->count + (int32_t)(((int64_t)diff * alpha_q8) >> 8);

        // ---- DEAD-BAND FOR OUTPUT ----
        if (u32AbsDiff(me->count, me->countOut) > me->outDeadBand) {
            me->countOut = me->count; // update output only if significant change
        }
    }

    return me->countOut;
}

////////////////////////////////////////////////////////////////////////////////

void xh17_setFilterParams(xh17Ctxt_t *me, int32_t dLow, int32_t dHigh,
                            uint8_t alphaMin_q8, uint8_t alphaMax_q8,
                            int32_t outDeadBand)
{
    me->dLow = dLow;
    me->dHigh = dHigh;
    me->alphaMin_q8 = alphaMin_q8;
    me->alphaMax_q8 = alphaMax_q8;
    me->outDeadBand = outDeadBand;
}

////////////////////////////////////////////////////////////////////////////////

int32_t xh17_readRawAvg(xh17Ctxt_t *me, uint8_t samples)
{
    uint32_t total = 0;
    for (uint8_t i = 0; i < samples; i++) {
        total += xh17_readRaw(me);
    }
    return (total / samples);
}

////////////////////////////////////////////////////////////////////////////////

void xh17_setScale(xh17Ctxt_t *me, uint32_t scale)
{
    me->scale = scale;
}

////////////////////////////////////////////////////////////////////////////////

void xh17_setOffset(xh17Ctxt_t *me, uint32_t offset)
{
    me->offset = offset;
}

////////////////////////////////////////////////////////////////////////////////

void xh17_tare(xh17Ctxt_t *me)
{
    xh17_setOffset(me, xh17_readFiltered(me));
}

////////////////////////////////////////////////////////////////////////////////

void xh17_setMode(xh17Ctxt_t *me, xh17_mode_t mode)
{
    if (mode == xh17_mode_PowerDown) {
        // Enter power-down mode
        pdSckSet_high(me);
        XH17_DELAY_US(65); // Hold PD_SCK high for at least 60us
    } else {
        // Exit power-down mode
        pdSckSet_low(me);
    }
}

////////////////////////////////////////////////////////////////////////////////

int16_t xh17_readRawUnits(xh17Ctxt_t *me)
{
    return (int16_t)((xh17_readRaw(me) - (int32_t)me->offset) / (int32_t)me->scale);
}

////////////////////////////////////////////////////////////////////////////////

int16_t xh17_readFilteredUnits(xh17Ctxt_t *me)
{
    return (int16_t)((xh17_readFiltered(me) - (int32_t)me->offset) / (int32_t)me->scale);
}
