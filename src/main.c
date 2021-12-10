#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "display_controller.h"
const uint8_t digit_to_7seg[] = {0x3f, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

int main()
{
	uint8_t command2[] = {0xC0, 0x06, 0xDB, 0x4F, 0x66, 0xED, 0xFD};
	uint16_t curr_num = 0;
	uint16_t curr_digit = 0;
	sei();
	DDRB |= 1 << PB5;
	PORTB |=1 << PB5;
	display_controller_init();
	while(1)
	{
		asm ("nop");
	}
	while (display_controller_ioctl(DISPLAY_CONTROLLER_IOCTL_IS_DONE, 0) == false)
	{
		asm ("nop");
	}
	display_controller_ioctl(DISPLAY_CONTROLLER_IOCTL_BRIGHTNESS, 8);
	while (display_controller_ioctl(DISPLAY_CONTROLLER_IOCTL_IS_DONE, 0) == false)
	{
		asm ("nop");
	}
	while(1)
	{
		display_controller_write(command2, sizeof(command2));
		curr_num++;
		for (uint8_t i = 1; i < sizeof(command2); i++)
		{
			curr_digit = curr_num;
			for (uint8_t j = 1; j < i; j++)
			{
				curr_digit /= 10;
			}
			curr_digit %= 10;
			command2[i] = digit_to_7seg[curr_digit];
			
		}
		_delay_ms(100);
	}
}

