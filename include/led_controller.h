#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "pin_configuration.h"

#define LED_CONTROLLER_SEGMENT_COUNT 6

typedef enum 
{
	BRIGHTNESS_1_16,
	BRIGHTNESS_2_16,
	BRIGHTNESS_4_16,
	BRIGHTNESS_10_16,
	BRIGHTNESS_11_16,
	BRIGHTNESS_12_16,
	BRIGHTNESS_13_16,
	BRIGHTNESS_14_16,
	BRIGHTNESS_FULL,
	BRIGHTNESS_NONE
} led_controller_brightness_t;

typedef struct 
{
	uint8_t segments[LED_CONTROLLER_SEGMENT_COUNT];
	uint8_t alarm_led;
} led_controller_data_t;


void led_controller_init();

void led_controller_write(const led_controller_data_t * data);

void led_controller_write_7seg(const uint8_t *segments);

void led_controller_write_alarm_led(const uint8_t data);

#endif