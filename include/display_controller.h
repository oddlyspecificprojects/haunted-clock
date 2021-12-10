#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "return_values.h"
#include "pin_configuration.h"

#define DISPLAY_CONTROLLER_DIGIT_COUNT 6

typedef struct 
{
	uint8_t segments[DISPLAY_CONTROLLER_DIGIT_COUNT];
} display_data_t;

// * 0b0abc - minutes, bottom minutes, top minutes
typedef enum DISPLAY_CONTROLLER_COLUMNS {
	DISPLAY_CONTROLLER_COLUMNS__SECONDS_TOP = 1,
	DISPLAY_CONTROLLER_COLUMNS__SECONDS_BOTTOM = 2,
	DISPLAY_CONTROLLER_COLUMNS__SECONDS = 3,
	DISPLAY_CONTROLLER_COLUMNS__MINUTES = 4,
	DISPLAY_CONTROLLER_COLUMNS__ALL = 7,
	DISPLAY_CONTROLLER_COLUMNS__ON,
	DISPLAY_CONTROLLER_COLUMNS__OFF,
	DISPLAY_CONTROLLER_COLUMNS__TOGGLE
};

typedef enum DISPLAY_CONTROLLER_IOCTL {
	DISPLAY_CONTROLLER_IOCTL_COLUMNS,
	DISPLAY_CONTROLLER_IOCTL_BRIGHTNESS,
	DISPLAY_CONTROLLER_IOCTL_IS_DONE,
};

uint8_t display_controller_init();

uint8_t display_controller_write(const void * data, uint8_t data_length);

uint8_t display_controller_ioctl(uint8_t action, uint8_t param);

#endif