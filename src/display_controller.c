#include "display_controller.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>


// WGM - wave generation mode, clear when counter == compare value A
#define TIMER_TCCRA (1<<WGM01)

// Compare value A to 19 (counts from 0-19 or 20 ticks)
//? Main DISPLAY_CONTROLLER_CLK is 16MHz, divided by 8 (prescaler) divided by 20, gives 100KHz interrupt frequency, DISPLAY_CONTROLLER_CLK is approx half of that
#define TIMER_OCRA (19)

// On compare, this interrupt occurs
#define TIMER_TIMSK (1<<OCIE0A)

// to set prescaler of 8, use this value
#define TIMER_PRESCALER (1<<CS01)

// TM in default mode
#define TM1637_CMD_INIT (0x40)

// Write from digit num 0
#define TM1637_CMD_WRITE (0xC0)

// TM misc display
#define TM1637_CMD_DISPLAY (0x80)

enum tm1637_state {INIT, START, CLOCK_FALL, DATA_WRITE, CLOCK_RISE, DATA_HOLD, ACK_SETUP, ACK_RISE, ACK_CHECK, ACK_RESET, STOP, END};

typedef struct 
{
	uint8_t curr_state;
	uint8_t curr_bit;
	uint8_t curr_byte;
	uint8_t buffer[DISPLAY_CONTROLLER_DIGIT_COUNT + 1]; // 6 digits + 1 command
	uint8_t buffer_len;
	uint8_t ack_count;

} state_t;

typedef void (*state_function_t)(void);

uint8_t s_shadow_buffer[DISPLAY_CONTROLLER_DIGIT_COUNT] = {0};
volatile state_t s_state = {0};
/*
 ? Protocol state machine functions
*/

static void state_func__init()
{
	DISPLAY_CONTROLLER_PORT |= DISPLAY_CONTROLLER_CLK | DISPLAY_CONTROLLER_DIO; 
	DISPLAY_CONTROLLER_OUT |= DISPLAY_CONTROLLER_CLK | DISPLAY_CONTROLLER_DIO;
	s_state.curr_state = START;
}


static void state_func__start()
{
	DISPLAY_CONTROLLER_OUT &= ~(DISPLAY_CONTROLLER_DIO);
	s_state.curr_state = CLOCK_FALL;
}


static void state_func__clock_fall()
{
	DISPLAY_CONTROLLER_OUT &= ~(DISPLAY_CONTROLLER_CLK);
	s_state.curr_state = DATA_WRITE;
}


static void state_func__data_write()
{
	if (s_state.buffer[s_state.curr_byte] & (1<<s_state.curr_bit))
	{
		DISPLAY_CONTROLLER_OUT |= DISPLAY_CONTROLLER_DIO;
	}
	else
	{
		DISPLAY_CONTROLLER_OUT &= ~DISPLAY_CONTROLLER_DIO;
	}
	s_state.curr_state = CLOCK_RISE;
}


static void state_func__clock_rise()
{
	DISPLAY_CONTROLLER_OUT |= DISPLAY_CONTROLLER_CLK;
	s_state.curr_state = DATA_HOLD;
}


static void state_func__data_hold()
{
	s_state.curr_bit++;
	if (s_state.curr_bit == 8)
	{
		s_state.curr_bit = 0;
		s_state.curr_state = ACK_SETUP;
	}
	else
	{
		s_state.curr_state = CLOCK_FALL;
	}
}


static void state_func__ack_setup()
{
	DISPLAY_CONTROLLER_OUT &= ~(DISPLAY_CONTROLLER_CLK | DISPLAY_CONTROLLER_DIO);
	DISPLAY_CONTROLLER_PORT &= ~(DISPLAY_CONTROLLER_DIO);
	DISPLAY_CONTROLLER_OUT |= DISPLAY_CONTROLLER_DIO;
	s_state.curr_state = ACK_RISE;
}



static void state_func__ack_rise()
{
	DISPLAY_CONTROLLER_OUT |= DISPLAY_CONTROLLER_CLK;
	s_state.curr_state = ACK_CHECK;
}


static void state_func__ack_check()
{
	if ((DISPLAY_CONTROLLER_IN & DISPLAY_CONTROLLER_DIO) == 0)
	{
		s_state.curr_state = ACK_RESET;
	}
	else
	{
		s_state.curr_state = ACK_CHECK;
	}
}


static void state_func__ack_reset()
{
	DISPLAY_CONTROLLER_OUT &= ~(DISPLAY_CONTROLLER_CLK | DISPLAY_CONTROLLER_DIO);
	DISPLAY_CONTROLLER_OUT |= DISPLAY_CONTROLLER_CLK | DISPLAY_CONTROLLER_DIO;
	s_state.curr_byte++;
	if (s_state.curr_byte < s_state.buffer_len)
	{
		s_state.curr_state = CLOCK_FALL;
	}
	else
	{
		s_state.curr_state = STOP;
	}
}


static void state_func__stop()
{
	DISPLAY_CONTROLLER_OUT &= ~(DISPLAY_CONTROLLER_DIO);
	s_state.curr_state = END;
}


static void state_func__end()
{
	DISPLAY_CONTROLLER_OUT |= DISPLAY_CONTROLLER_CLK | DISPLAY_CONTROLLER_DIO;
	TCCR0B = 0; // stop the timer
	s_state.curr_state = INIT;
}


/*
 ? Timer functions
*/
static void timer_init()
{
	TCCR0A = TIMER_TCCRA;
	TCNT0 = 0;
	OCR0A = TIMER_OCRA;
	TIMSK0 = TIMER_TIMSK;
}

static void timer_start()
{
	TCCR0B = TIMER_PRESCALER;
}

/*
 ? Display managment functions
*/

static bool is_busy()
{
	return s_state.curr_state != INIT;
}

static uint8_t state_buffer_init(uint8_t command, const void * data, uint8_t data_length)
{
	if (is_busy())
	{
		return RVAL_BUSY;
	}
	if (data_length > DISPLAY_CONTROLLER_DIGIT_COUNT)
	{
		return RVAL_INSUFFICIENT_BUFFER;
	}
	memset(&s_state, 0, sizeof(s_state));
	s_state.buffer[0] = command;
	if (data_length > 0 && data != NULL)
	{
		memcpy(s_state.buffer + 1, data, data_length);
	}
	s_state.buffer_len = data_length + 1;
	return RVAL_SUCCESS;
}

static uint8_t display_send_command(uint8_t command, const void * data, uint8_t data_length)
{
	uint8_t retval = state_buffer_init(command, data, data_length);
	if (retval != RVAL_SUCCESS)
	{
		return retval;
	}
	
	timer_start();
	
	return retval;
}

static uint8_t ioctl_is_done()
{
	return is_busy() == false;
}

static void set_column(uint8_t * digit, uint8_t val)
{
	if (val == DISPLAY_CONTROLLER_COLUMNS__ON)
	{
		*digit |= val << 7;
	}
	else if (val == DISPLAY_CONTROLLER_COLUMNS__OFF)
	{
		*digit &= ~(val << 7);
	}
	else
	{
		*digit ^= val << 7;
	}
}

static uint8_t ioctl_columns(uint8_t param)
{
	uint8_t column_val = param & (~DISPLAY_CONTROLLER_COLUMNS__ALL);
	if (is_busy())
	{
		return RVAL_BUSY;
	}
	if (param & DISPLAY_CONTROLLER_COLUMNS__SECONDS_TOP)
	{
		set_column(s_shadow_buffer, column_val);
	}
	if (param & DISPLAY_CONTROLLER_COLUMNS__SECONDS_BOTTOM)
	{
		set_column(s_shadow_buffer + 1, column_val);
	}
	if (param & DISPLAY_CONTROLLER_COLUMNS__MINUTES)
	{
		set_column(s_shadow_buffer + 3, column_val);
	}
	return display_send_command(TM1637_CMD_WRITE, s_shadow_buffer, sizeof(s_shadow_buffer));	
}

static uint8_t ioctl_brightness(uint8_t param)
{
	uint8_t out_val = TM1637_CMD_DISPLAY;
	if (param > 8)
	{
		param = 7;
	}
	if (param > 0)
	{
		// bit 3 (0x08) is display on/off, if display on, all 3 bottom bits are used for brightness, and all 3 lower bits 0 also mean display is on
		out_val |= 0x08 | (param-1);
	}
	return display_send_command(out_val, NULL, 0);
	
}

uint8_t display_controller_init()
{
	memset(&s_state, 0, sizeof(s_state));
	if (is_busy())
	{
		
		return RVAL_BUSY;
	}
	timer_init();
	
	return display_send_command(TM1637_CMD_INIT, NULL, 0);
}



uint8_t display_controller_write(const void * data, uint8_t data_length)
{
	return display_send_command(TM1637_CMD_WRITE, data, data_length);	
}


uint8_t display_controller_ioctl(uint8_t action, uint8_t param)
{
	uint8_t ret_val = 0;
	switch (action)
	{
	case DISPLAY_CONTROLLER_IOCTL_COLUMNS:
		ret_val = ioctl_columns(param);
		break;
	
	case DISPLAY_CONTROLLER_IOCTL_BRIGHTNESS:
		ret_val = ioctl_brightness(param);
		break;
	
	case DISPLAY_CONTROLLER_IOCTL_IS_DONE:
		ret_val = ioctl_is_done();
		break;
	}

	return ret_val;
}


ISR(TIMER0_COMPA_vect)
{
	switch(s_state.curr_state)
	{
		case INIT:
			state_func__init();
			break;

		case START:
			state_func__start();
			break;

		case CLOCK_FALL:
			state_func__clock_fall();
			break;

		case DATA_WRITE:
			state_func__data_write();
			break;

		case CLOCK_RISE:
			state_func__clock_rise();
			break;

		case DATA_HOLD:
			state_func__data_hold();
			break;

		case ACK_SETUP:
			state_func__ack_setup();
			break;

		case ACK_RISE:
			state_func__ack_rise();
			break;

		case ACK_CHECK:
			state_func__ack_check();
			break;

		case ACK_RESET:
			state_func__ack_reset();
			break;

		case STOP:
			state_func__stop();
			break;

		case END:
			state_func__end();
			break;
	}
	
}
