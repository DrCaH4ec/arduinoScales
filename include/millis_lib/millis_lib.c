#include "millis_lib.h"

volatile uint32_t timer0_overflow_count = 0;
volatile uint32_t timer0_millis = 0;
static uint8_t timer0_fract = 0;


ISR(TIMER0_OVF_vect)
{
	uint32_t m = timer0_millis;
	uint8_t f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;

	if(f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m++;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void millis_init()
{
	TCCR0B |= (1 << CS01) | (1 << CS00);
	TIMSK0 = (1 << TOIE0);
	TCCR0A = 0x00;
	TCNT0 = 0x00;

	sei();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t micros()
{
	uint32_t m;
	uint8_t old_SREG = SREG;
	uint8_t t;

	cli();
	m = timer0_overflow_count;
	t = TCNT0;

	if ((TIFR0 & (1 << TOV0)) && (t < 255))
		m++;

	SREG = old_SREG;

	return ((m << 8) + t) * (64 / CLOCK_CYCLES_PER_USEC());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t millis()
{
	uint32_t m;
	uint8_t old_SREG = SREG;

	cli();
	m = timer0_millis;

	SREG = old_SREG;

	return m;
}