// ****************************************************************************
/*
	Project:	preamp
	Purpose:	Declaration of RC5 functions & stuff
*/
// ****************************************************************************
#ifndef __PREAMP_RC5_H__
#define __PREAMP_RC5_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#define RC5TIME 	1.778e-3		// 1.778msec
#define PULSE_MIN	(uint8_t)(XTAL / 512 * RC5TIME * 0.4 + 0.5)
#define PULSE_1_2	(uint8_t)(XTAL / 512 * RC5TIME * 0.8 + 0.5)
#define PULSE_MAX	(uint8_t)(XTAL / 512 * RC5TIME * 1.2 + 0.5)

struct rc5_state
{
	uint8_t		rc5_bit;				// bit value
	uint8_t		rc5_time;				// count bit time
	uint16_t	rc5_tmp;				// shift bits in
	uint16_t	rc5_data;				// store result
};

    
struct rc5_packet
{
    uint8_t valid:1;
    uint8_t toggle:1;
    uint8_t address:5;
    uint8_t code:7;
};

/**
 *  Decode RC5 packet
 */
void rc5_packet_decode( uint16_t value, struct rc5_packet* packet );
    
/**
 *  Key definitions
 */
    
#define EXTRC5(x) (0x40 | x)

#define IR_KEY_1      0x01
#define IR_KEY_2      0x02
#define IR_KEY_3      0x03
#define IR_KEY_4      0x04
#define IR_KEY_5      0x05
#define IR_KEY_6      0x06

#define IR_KEY_LEFT    0x11
#define IR_KEY_RIGHT   0x10
#define IR_KEY_UP      0x20
#define IR_KEY_DOWN    0x21
    
#define IR_KEY_MENU     EXTRC5(0x34)
#define IR_KEY_EXIT     0x3C

#define IR_KEY_RED      EXTRC5(0x35)
#define IR_KEY_GREEN    0x0F
#define IR_KEY_YELLOW   EXTRC5(0x32)
#define IR_KEY_BLUE     EXTRC5(0x31)

#define IR_KEY_OK    0x29

#define IR_KEY_MUTE  0x0d

#define IR_KEY_POWER    0x0c
    
    
    
#ifdef __cplusplus
}
#endif
        
#endif // __PREAMP_RC5_H__
