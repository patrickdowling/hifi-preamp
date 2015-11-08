// ***************************************************************************
/*
 Project:	Preamp
 Purpose:	Rotary encoder declarations and helper functions
 */
// ***************************************************************************
#ifndef __PREAMP_ENCODER_H__
#define __PREAMP_ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#define ENCODER_CONV( x )	( x )
#define ENCODER_MAX		(255)

typedef uint8_t	encoder_value_t;	// uint16_t for, ALPS STEC11B counts 2 for each step, so we count 0-512

struct encoder_state
{
	encoder_value_t	value;	
	uint8_t		status;
	uint8_t		step;
};
    
#ifdef __cplusplus
}
#endif
        
#endif // __PREAMP_ENCODER_H__
