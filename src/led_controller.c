#include "led_controller.h"

static void start_cond()
{
	
}

void led_controller_init()
{
	// init the pins
	LED_CONTROLLER_ALARM_LED_PORT |= LED_CONTROLLER_ALARM_LED;
	LED_CONTROLLER_7SEG_PORT |= LED_CONTROLLER_7SEG_DIO | LED_CONTROLLER_7SEG_CLK;
	// Default state for led is off
	LED_CONTROLLER_ALARM_LED_OUT &= ~(LED_CONTROLLER_ALARM_LED);
	// Default state for communication pins is high
	LED_CONTROLLER_7SEG_OUT |= LED_CONTROLLER_7SEG_DIO | LED_CONTROLLER_7SEG_CLK;

}

void led_controller_write(const led_controller_data_t * data);

void led_controller_write_7seg(const uint8_t *segments);

void led_controller_write_alarm_led(const uint8_t data);
