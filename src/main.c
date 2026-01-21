#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include <stdint.h>
#include <stdlib.h>

#include "xh17_lib.h"
#include "tm1637_lib.h"
#include "usart_lib.h"
#include "millis_lib.h"
#include "button_lib.h"

#define CALIBRATION_WEIGHT 1000


XH17_DECLARE_CTXT(scaler, PORTD, 5, PORTD, 6);
TM16_DECLARE_CTXT(disp, PORTD, 4, PORTD, 3, 4);
BUTTON_DECLARE_CTXT(buttonTare, PORTB, 0, 0, 1);
BUTTON_DECLARE_CTXT(buttonScale, PORTD, 2, 0, 1);

uint8_t EEMEM scaleVal = 1;

int main(void) {
    USART0_init();
    millis_init();

    tm1637_initHw(&disp);
    tm1637_setBrightness(&disp, 2);

    xh17_initHw(&scaler);
    xh17_setInputSelect(&scaler, xh17_inputSelect_A_64);
    xh17_tare(&scaler);
    scaler.scale = eeprom_read_byte(&scaleVal);
    xh17_setScale(&scaler, scaler.scale);

    button_initHw(&buttonTare);
    button_initHw(&buttonScale);
    
    int16_t weight, prevWeight = 0;

    tm1637_print(&disp, "v01");
    _delay_ms(2000);
    tm1637_print(&disp, "0000");

    while (1) {

        if (button_isPressed(&buttonTare)) {
            xh17_tare(&scaler);
        }

        if (button_isPressed(&buttonScale)) {
            uint32_t load = xh17_readFiltered(&scaler);

            scaler.scale = (load - scaler.offset) / CALIBRATION_WEIGHT;
            xh17_setScale(&scaler, scaler.scale);
            eeprom_write_byte(&scaleVal, scaler.scale);
        }

        if (xh17_isReady(&scaler)) {
            char buffer[32];
            // int16_t raw = xh17_readRawUnits(&scaler);

            weight = xh17_readFilteredUnits(&scaler);
            if (weight != prevWeight) {
                snprintf(buffer, sizeof(buffer), "%d;", (weight/10)*10);
                USART0_SendData(buffer);

                snprintf(buffer, sizeof(buffer), "%02d%02d", weight/1000, (weight%1000)/10);
                tm1637_print(&disp, buffer);

                prevWeight = weight;
            }
        }
    }

    return 0;
}
