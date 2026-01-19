#ifndef _XH17_LIB_H_
#define _XH17_LIB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

/* Default values for adaptive filter parameters */
#define XH17_ALPHA_MIN_Q8_DEFAULT   32
#define XH17_ALPHA_MAX_Q8_DEFAULT   128
#define XH17_OUT_DEAD_BAND_DEFAULT  1000
#define XH17_D_LOW_DEFAULT          1500
#define XH17_D_HIGH_DEFAULT         15000

typedef enum {
    xh17_inputSelect_A_128 = 0,
    xh17_inputSelect_B_32,
    xh17_inputSelect_A_64
} xh17_inputSelect_t;

typedef enum {
    xh17_mode_Normal = 0,
    xh17_mode_PowerDown
} xh17_mode_t;

typedef struct {
    volatile uint8_t *pdSckDDR;
    volatile uint8_t *pdSckPORT;
    uint8_t pdSckBIT;

    volatile uint8_t *dOutDDR;
    volatile uint8_t *dOutPORT;
    volatile uint8_t *dOutPIN;
    uint8_t dOutBIT;

    uint32_t offset;
    uint32_t scale;

    xh17_inputSelect_t inputSelect;

    /* Adaptive filter state */
    int32_t count;         // internal EMA state
    int32_t countOut;      // stabilized output
    uint8_t filtInited;     // filter initialization flag

    // Dead-band on output to avoid small fluctuations
    int32_t outDeadBand;

    /* Adapation thresholds for |x - y| (raw counts)
    dLow: zone "almost noise" -> alphaMin_q8
    dHigh: zone "large change" -> alphaMax_q8 */
    int32_t dLow;
    int32_t dHigh;
    uint8_t alphaMin_q8;
    uint8_t alphaMax_q8;
} xh17Ctxt_t;

#define XH17_DECLARE_CTXT(name, pdSckPort, pdSckBit, dOutPort, dOutBit) \
    xh17Ctxt_t name = { \
        /* DDR register is PORT - 1 */ \
        /* PIN register is PORT - 2 */ \
        .pdSckPORT = &(pdSckPort), \
        .pdSckDDR = &(pdSckPort) - 1, \
        .pdSckBIT = (pdSckBit), \
        .dOutPORT = &(dOutPort), \
        .dOutDDR = &(dOutPort) - 1, \
        .dOutPIN = &(dOutPort) - 2, \
        .dOutBIT = (dOutBit), \
        .offset = 0, \
        .scale = 1, \
        .inputSelect = xh17_inputSelect_A_128, \
        .count = 0, \
        .countOut = 0, \
        .filtInited = 0, \
        .alphaMin_q8 = XH17_ALPHA_MIN_Q8_DEFAULT, \
        .alphaMax_q8 = XH17_ALPHA_MAX_Q8_DEFAULT, \
        .outDeadBand = XH17_OUT_DEAD_BAND_DEFAULT, \
        .dLow = XH17_D_LOW_DEFAULT, \
        .dHigh = XH17_D_HIGH_DEFAULT \
    };

#define XH17_DELAY_US(us) _delay_us(us) // Placeholder for delay function

/**
 * @fn xh17_initHw
 * @param me     - Pointer to the XH17 context structure.
 * @brief Initialize the XH17 sensor interface.
 */
void xh17_initHw(xh17Ctxt_t *me);

/**
 * @fn xh17_isReady
 * @param me     - Pointer to the XH17 context structure.
 * @brief Check if the XH17 sensor is ready to send data.
 * @return true if ready, false otherwise.
 */
bool xh17_isReady(xh17Ctxt_t *me);

/**
 * @fn xh17_waitUntilReady
 * @param me     - Pointer to the XH17 context structure.
 * @brief Wait until the XH17 sensor is ready to send data.
 */
void xh17_waitUntilReady(xh17Ctxt_t *me);

/**
 * @fn xh17_readRaw
 * @param me     - Pointer to the XH17 context structure.
 * @brief Read raw data from the XH17 sensor.
 * @return 24-bit signed raw data.
 */
int32_t xh17_readRaw(xh17Ctxt_t *me);

/**
 * @fn xh17_readRawAvg
 * @param me     - Pointer to the XH17 context structure.
 * @param samples   - Number of samples to average.
 * @brief Read average raw data from the XH17 sensor.
 * @return 24-bit signed average raw data.
 */
int32_t xh17_readRawAvg(xh17Ctxt_t *me, uint8_t samples);

/**
 * @fn xh17_setInputSelect
 * @param me         - Pointer to the XH17 context structure.
 * @param inputSelect   - Input selection mode.
 * @brief Set the input selection mode for the XH17 sensor.
 */
void xh17_setInputSelect(xh17Ctxt_t *me, xh17_inputSelect_t inputSelect);

/**
 * @fn xh17_readFiltered
 * @param me     - Pointer to the XH17 context structure.
 * @brief Read adaptively filtered data from the XH17 sensor.
 * @return Filtered data.
*/
int32_t xh17_readFiltered(xh17Ctxt_t *me);

/**
 * @fn xh17_tare
 * @param me - Pointer to the XH17 context structure.
 * @brief Set the current filtered reading as the offset (tare).
 */
void xh17_tare(xh17Ctxt_t *me);

/**
 * @fn xh17_setScale
 * @param me    - Pointer to the XH17 context structure.
 * @param scale   - Scale factor to apply to readings.
 * @brief Set the scale factor for the XH17 sensor readings.
 */
void xh17_setScale(xh17Ctxt_t *me, uint32_t scale);

/**
 * @fn xh17_setOffset
 * @param me     - Pointer to the XH17 context structure.
 * @param offset   - Offset value to set.
 * @brief Set the offset for the XH17 sensor readings.
 */
void xh17_setOffset(xh17Ctxt_t *me, uint32_t offset);

/**
 * @fn xh17_setFilterParams
 * @param me             - Pointer to the XH17 context structure.
 * @param dLow           - Lower threshold for adaptive filtering.
 * @param dHigh          - Upper threshold for adaptive filtering.
 * @param alphaMin_q8    - Minimum alpha value in Q8 format.
 * @param alphaMax_q8    - Maximum alpha value in Q8 format.
 * @param outDeadBand    - Dead-band for output stabilization.
 * @brief Set the parameters for the adaptive filter.
 */
void xh17_setFilterParams(xh17Ctxt_t *me, int32_t dLow, int32_t dHigh,
                            uint8_t alphaMin_q8, uint8_t alphaMax_q8,
                            int32_t outDeadBand);

/**
 * @fn xh17_setMode
 * @param me     - Pointer to the XH17 context structure.
 * @param mode   - Operating mode to set.
 * @brief Set the operating mode of the XH17 sensor.
 */
void xh17_setMode(xh17Ctxt_t *me, xh17_mode_t mode);

/**
 * @fn xh17_readRawUnits
 * @param me     - Pointer to the XH17 context structure.
 * @brief Read raw data in units from the XH17 sensor.
 */
int16_t xh17_readRawUnits(xh17Ctxt_t *me);

/**
 * @fn xh17_readFilteredUnits
 * @param me     - Pointer to the XH17 context structure.
 * @brief Read filtered data in units from the XH17 sensor.
 */
int16_t xh17_readFilteredUnits(xh17Ctxt_t *me);

/* _XH17_LIB_H_ */
#endif
