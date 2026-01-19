#ifndef _USART0_H_
#define _USART0_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//-----------------------------------------------------------------------------
//#############################################################################
//-----------------------------------------------------------------------------

// Setting of parity checking
// 0b00	->	disabled
// 0b01	->	reserved(don't use)
// 0b10 ->	enabled, even parity
// 0b11 ->	enabled, odd parity
#define USART0_PARITY_MODE 0

// Enabling/disabling of double speed mode
// 1 -> double speed mode is enable
// 0 -> double speed mode is disable
#define USART0_SPEED_MODE	1

// Setting number of bits in one "piece" of data
// valid values is 5, 6, 7, 8(default)
#define USART0_DATA_SIZE 8

// Number of stop bits
// 0 -> 1 stop bit
// 1 -> 2 stop bits
#define USART0_STOP_BIT 2

// Speed of USART0 in bod
#define USART0_BAUD_RATE 115200

// Enabling/disabling of receiving data
// 1 -> receiving data is enable
// 0 -> receiving data is disable
#define USART0_RX_EN 0

// Enabling/disabling of transmitting data
// 1 -> transmitting data is enable
// 0 -> transmitting data is disable
#define USART0_TX_EN 1

// Enabling/disabling of interrupt for receiver
// 1 -> interrupt for receiver is enable
// 0 -> interrupt for receiver is disable
#define USART0_RXCI_EN 0


//#define USART0_START_SYMDOL	'#'
#define USART0_STOP_SYMBOL 	';'

//Size of receiving buffer
#define USART0_BUFFER_SIZE 16

#define USART0_TIMEOUT 10


//-----------------------------------------------------------------------------
//#############################################################################
//-----------------------------------------------------------------------------

enum USART0_STATUS{
	READY_FOR_NEW_DATA = 0,
	RECEIVED_OK,
	FRAME_ERROR,
	DATA_OVER_RUN,
	USART0_PARITY_ERROR
};

//-----------------------------------------------------------------------------
//#############################################################################
//-----------------------------------------------------------------------------


/**
 * Initialization of USART0. All necessary settings you need to set in defines
 */
void USART0_init();


/**
 * Sending 1 character to USART
 * @param data ASCII or another code of character
 */
void USART0_SendChar(char data);


/**
 * Send string by USART
 * @param data Pointer to first element of array of characters
 */
void USART0_SendData(char * data);


/**
 * Return status of receiving of data
 * @return [description]
 */
uint8_t USART0_GetStatus();


/**
 * Transferring of data from internal buffer of library to user's buffer
 * @param data Pointer to first element of array-destination
 */
void USART0_ReadBuffer(char * data);


/**
 * "Says" the program that data was read and enable receiving of new data
 */
void USART0_DataWasRead();

/*_USART0_H_*/
#endif
