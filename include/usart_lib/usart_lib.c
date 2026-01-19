#include "usart_lib.h"

volatile char USART0_buf[USART0_BUFFER_SIZE];

enum USART0_STATUS USART0_Status = READY_FOR_NEW_DATA;

volatile uint8_t counter = 0;


#ifdef USART0_START_SYMDOL
	uint8_t ReceiveEnable = 0;
#else
	uint8_t ReceiveEnable = 1;
#endif


ISR(USART_RX_vect)
{
	cli();

	char tmp = USART0_STOP_SYMBOL;

	uint16_t loop = USART0_TIMEOUT;

	do 
	{
		if(UCSR0A&(1<<RXC0))
		{
			tmp = UDR0;
			break;			
		}
		_delay_ms(1);		
	} while (--loop);


	if(ReceiveEnable == 1){	

		if(tmp == USART0_STOP_SYMBOL || !(counter < USART0_BUFFER_SIZE)){
			USART0_buf[counter] = '\0';
			ReceiveEnable = 0;
			counter = 0;
			USART0_Status = RECEIVED_OK;
		}
		else{
			USART0_buf[counter] = tmp;		
			counter++;
		}
	}


#ifdef USART0_START_SYMDOL
	if(tmp == USART0_START_SYMDOL)
		ReceiveEnable = 1;
#endif

	sei();
}

//-----------------------------------------------------------------------------


void USART0_init()
{
	uint32_t USARTSpeed;

	if(USART0_SPEED_MODE){
		UCSR0A |= (1<<U2X0);
		USARTSpeed = (F_CPU / 8 / USART0_BAUD_RATE) - 1;
	}
	else{
		UCSR0A &= ~(1<<U2X0);
		USARTSpeed = (F_CPU / 16 / USART0_BAUD_RATE) - 1;
	}

	UBRR0H = (uint8_t)(USARTSpeed >> 8);
	UBRR0L = (uint8_t)USARTSpeed;


	UCSR0B = USART0_RX_EN ? UCSR0B|(1<<RXEN0) : UCSR0B&(~(1<<RXEN0));

	UCSR0B = USART0_TX_EN ? UCSR0B|(1<<TXEN0) : UCSR0B&(~(1<<TXEN0));

	UCSR0C = USART0_STOP_BIT ?  UCSR0C|(1<<USBS0) : UCSR0C&(~(1<<USBS0));

	UCSR0B = USART0_RXCI_EN ? UCSR0B|(1<<RXCIE0) : UCSR0B&(~(1<<RXCIE0));	


	UCSR0C &= ~(1<<UPM00);
	UCSR0C &= ~(1<<UPM01);

	UCSR0C = USART0_PARITY_MODE&(1<<0) ?  UCSR0C|(1<<UPM00) : UCSR0C;
	UCSR0C = USART0_PARITY_MODE&(1<<1) ?  UCSR0C|(1<<UPM01) : UCSR0C;


	switch(USART0_DATA_SIZE){
		case 5:
			UCSR0C &= ~(1<<UCSZ00);
			UCSR0C &= ~(1<<UCSZ01);
			UCSR0B &= ~(1<<UCSZ02);
		break;

		case 6:
			UCSR0C |= (1<<UCSZ00);
			UCSR0C &= ~(1<<UCSZ01);
			UCSR0B &= ~(1<<UCSZ02);
		break;

		case 7:
			UCSR0C &= ~(1<<UCSZ00);
			UCSR0C |= (1<<UCSZ01);
			UCSR0B &= ~(1<<UCSZ02);
		break;

		case 8:
			UCSR0C |= (1<<UCSZ00);
			UCSR0C |= (1<<UCSZ01);
			UCSR0B &= ~(1<<UCSZ02);
		break;

		case 9:
			UCSR0C |= (1<<UCSZ00);
			UCSR0C |= (1<<UCSZ01);
			UCSR0B |= (1<<UCSZ02);
		break;

		default:
			UCSR0C &= ~(1<<UCSZ00);
			UCSR0C |= (1<<UCSZ01);
			UCSR0B |= (1<<UCSZ02);		
		break;
	}	


	while ( !( UCSR0A & (1<<UDRE0)) );	
}


//-----------------------------------------------------------------------------


void USART0_SendChar(char data)
{
	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
}


//-----------------------------------------------------------------------------


void USART0_SendData(char * data)
{
//	uint16_t i;
	// for(i = 0; data[i] != '\0'; i++)
	// 	USART0_SendChar(data[i]);
	while(*data != '\0')
		USART0_SendChar(*data++);

}


//-----------------------------------------------------------------------------


uint8_t USART0_GetStatus()
{
	return USART0_Status;
}


//-----------------------------------------------------------------------------


void USART0_DataWasRead()
{
	USART0_Status = READY_FOR_NEW_DATA;
	ReceiveEnable = 1;
}


//-----------------------------------------------------------------------------


void USART0_ReadBuffer(char * data)
{
	uint8_t i = 0;

	while(USART0_buf[i] != '\0' && i < USART0_BUFFER_SIZE){
		data[i] = USART0_buf[i];
		i++;
	}

	data[i] = '\0';

	i = USART0_BUFFER_SIZE;

	while(--i > 0)
		USART0_buf[i] = ' ';
}
