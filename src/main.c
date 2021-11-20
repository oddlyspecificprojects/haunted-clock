#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <string.h>
volatile uint8_t data = 0;
volatile uint8_t done = 0;
volatile uint8_t interrupt_count = 0;

#define CLOCK	(1<<PD5)
#define DIO		(1<<PD6)


enum tm1637_state {INIT, START, CLOCK_FALL, DATA_WRITE, CLOCK_RISE, DATA_HOLD, ACK_SETUP, ACK_RISE, ACK_CHECK, ACK_RESET, STOP, END};
typedef struct 
{
	uint8_t curr_state;
	uint8_t curr_bit;
	uint8_t curr_byte;
	uint8_t buffer[7];
	uint8_t buffer_len;
	uint8_t ack_count;

} state_t;

volatile state_t state = {0};


void tick()
{
	//PORTB ^= (1<<PB5);
	for (uint8_t i = 0; i < 55; i++)
	{
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}
	
}

void init()
{
	PORTD = CLOCK | DIO;
	state.curr_state = START;
}

void start()
{
	PORTD = CLOCK;
	state.curr_state = CLOCK_FALL;
}

void clock_fall()
{
	PORTD &= ~(CLOCK);
	state.curr_state = DATA_WRITE;
}


void data_write()
{
	PORTD = (state.buffer[state.curr_byte] & (1<<state.curr_bit)) ? DIO : 0;
	state.curr_state = CLOCK_RISE;
}

void clock_rise()
{
	PORTD |= CLOCK;
	state.curr_state = DATA_HOLD;
}

void data_hold()
{
	state.curr_bit++;
	if (state.curr_bit == 8)
	{
		state.curr_bit = 0;
		state.curr_state = ACK_SETUP;
	}
	else
	{
		state.curr_state = CLOCK_FALL;
	}
}

void ack_setup()
{
	PORTD = 0;
	DDRD = CLOCK;
	PORTD = DIO;
	state.curr_state = ACK_RISE;
}



void ack_rise()
{
	PORTD |= CLOCK;
	state.curr_state = ACK_CHECK;
}
void ack_check()
{
	if ((PIND & DIO) == 0)
	{
		state.curr_state = ACK_RESET;
	}
	else
	{
		state.curr_state = ACK_CHECK;
	}
}

void ack_reset()
{
	PORTD = 0;
	DDRD = CLOCK | DIO;
	state.curr_byte++;
	if (state.curr_byte < state.buffer_len)
	{
		state.curr_state = CLOCK_FALL;
	}
	else
	{
		state.curr_state = STOP;
	}
}

void stop()
{
	PORTD = CLOCK;
	state.curr_state = END;
}

void end()
{
	PORTD = CLOCK | DIO;
	TCCR0B = 0; // stop the timer
	state.curr_state = INIT;
}

void tm1637_state_machine()
{
	switch (state.curr_state)
	{
	case INIT:
		init();
		break;
	
	case START:
		start();
		break;
	
	case CLOCK_FALL:
		clock_fall();
		break;

	case DATA_WRITE:
		data_write();
		break;
	
	case CLOCK_RISE:
		clock_rise();
		break;
	
	case DATA_HOLD:
		data_hold();
		break;
	
	case ACK_SETUP:
		ack_setup();
		break;
	
	case ACK_RISE:
		ack_rise();
		break;

	case ACK_CHECK:
		ack_check();
		break;
	
	case ACK_RESET:
		ack_reset();
		break;
	
	case STOP:
		stop();
		break;
	
	case END:
		end();
		break;

	default:
		break;
	}
}

void send_data(const uint8_t * data, uint8_t data_len)
{
	memset(&state, 0, sizeof state);
	if (data_len > 7)
	{
		return;
	}
	memcpy(state.buffer, data, data_len);
	state.buffer_len = data_len;
	TCCR0B = 2; // start the timer
}

int main()
{
	const uint8_t command1 = 0x40;
	const uint8_t command3 = 0x89;
	uint8_t command2[] = {0xC0, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D};
	//send_params(command1, command2, command3);
	// output + high
	memset(&state, 0, sizeof(state));
	
	sei();
	DDRB = 1<<PB5;
	DDRD = DIO | CLOCK;
	PORTD = DIO | CLOCK;
	tick();
	tick();
	tick();
	// OUTA toggle on match, CTC on OCR0A
	TCCR0A = (1<<WGM01);
	TCNT0 = 0;
	OCR0A = 19; // 16,000,000 / 64 = 250,000 / 40 = 6250 / 2 = 3125Hz, F_CPU/PRESCALER/COUNT/TOGGLE
	TIMSK0 = 1 << OCIE0A; // interrupt on B
	
	send_data(&command1, 1);
	while (state.curr_state != END)
	{
		asm("nop");
	}
	send_data(command2, sizeof(command2));
	while(state.curr_state != END)
	{
		asm("nop");
	}
	send_data(&command3, 1);
	while(1)
	{
		asm("nop");
	}
}

ISR(TIMER0_COMPA_vect)
{
	tm1637_state_machine();
}
