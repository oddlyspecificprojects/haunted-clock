#ifndef RETURN_VALUES_H
#define RETURN_VALUES_H

typedef enum {
	RVAL_SUCCESS,
	RVAL_BUSY,
	RVAL_ALREADY_INIT,
	RVAL_INSUFFICIENT_BUFFER,
} return_values_t;

#endif