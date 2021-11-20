#ifndef PIN_CFG_H
#define PIN_CFG_H

#include <avr/io.h>

// LED controller - 7 segment and alarm led
#define LED_CONTROLLER_7SEG_CLK			(1 << PB0)
#define LED_CONTROLLER_7SEG_DIO			(1 << PB1)
#define LED_CONTROLLER_7SEG_PORT		(PORTB)
#define LED_CONTROLLER_7SEG_OUT			(DDRB)
#define LED_CONTROLLER_7SEG_IN			(PINB)
#define LED_CONTROLLER_ALARM_LED		(1 << PD3)
#define LED_CONTROLLER_ALARM_LED_PORT	(PORTB)
#define LED_CONTROLLER_ALARM_LED_OUT	(DDRB)
#define LED_CONTROLLER_ALARM_LED_IN		(PINB)

#endif