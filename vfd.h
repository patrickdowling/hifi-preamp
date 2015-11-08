// ***************************************************************************
/*
 Project:	Preamp
 Purpose:	Declare VFD related functions & defines
 */
// ***************************************************************************

#ifndef __PREAMP_VFD_H__
#define __PREAMP_VFD_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stddef.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#include "icons.h"


#define VFDOUT( x ) VFD_PORT &= ~VFD_SS; SPIOUT( x ); VFD_PORT |= VFD_SS; _delay_us(6); while( VFD_PIN & VFD_MB );

#define VFDCOORD( x, y ) { const uint16_t _x_ = x; \
VFDOUT( (uint8_t)( _x_ >> 8) ); VFDOUT( (uint8_t)( _x_ )  ); \
VFDOUT( y ); \
}

//
// Commands
//
#define VFD_CLEAR	0x01
#define VFD_HOME	0x02
#define VFD_SETFONT	0xF2
#define VFD_SETLUM	0x30
#define VFD_ON          0x08 | ( 0x01 << 2 )
#define VFD_OFF         0x08

#define VFD_RES_X	280
#define VFD_RES_Y	16
    
#define VFD_DRAW_CLEAR      'C'
#define VFD_DRAW_OUTLINE    'O'
#define VFD_DRAW_FILL       'F'
#define VFD_DRAW_INVERT     'I'
    
//
// Screen positions
//
#define ICON_AREA_X		0
#define ICON_AREA_W     20
    
#define	VOLUME_X	195
#define VOLUME_Y	15
    
#define MIDDLE_AREA_X       ( ICON_AREA_X + ICON_AREA_W + 2 )
#define MIDDLE_AREA_W       ( VOLUME_X - MIDDLE_AREA_X - 2 )
    
#define FONT_5x7	'B'
#define FONT_10x14	'C'
#define FONT_MINI	'A'

#define INPUT_FIELD_X   MIDDLE_AREA_X
#define INPUT_TEXT_X	( INPUT_FIELD_X + 2 )
#define INPUT_CHAR_X_OFFSET	3
#define INPUT_CHAR_Y_OFFSET 7
#define INPUT_CHAR_W	10

#define INPUT_NAME_X	0	// 64
#define INPUT_NAME_Y	15

#define INPUT_LABEL_X   (INPUT_FIELD_X + 4 * INPUT_CHAR_W)

#define VOLUME_BAR_X    ( MIDDLE_AREA_X )
#define VOLUME_BAR_W    ( MIDDLE_AREA_W - 2 )
    
    
//
// Stuff
//

/**
 * Initialize VFD display
 */
void vfd_init( void );

/**	
 * Issue command to VFD display
 * @param	cmd	command to issue
 */
void vfd_cmd( const uint8_t cmd );

/**
 * Issue command with parameter
 */
void vfd_cmd_param( const uint8_t cmd, const uint8_t param );

/**
 * Output zero-terminated string to VFD
 */
void vfd_string( const char * str );
void vfd_string_p( const prog_char str[] );

/**
 * Output zero-terminated string to position on VFD
 */
void vfd_string_at( const uint16_t x, const uint8_t y, const char * str );
void vfd_string_at_p( const uint16_t x, const uint8_t y, const prog_char str[] );

/**
 * Output raw data to VFD
 */
void vfd_data( const uint8_t * data, size_t len );
void vfd_data_p( const uint8_t * data, size_t len );

/**
 * Issue draw command
 */
void vfd_draw( const uint16_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint8_t cmd );

/**
 * Set VFD cursor position
 */
void vfd_set_cursor( const uint8_t x, const uint8_t y );

/**
 *  Draw icon
 */
void vfd_draw_icon( const uint16_t x, const uint8_t y, const struct icon_descriptor* pIcon );

    
#ifdef __cplusplus
}
#endif

#endif // __PREAMP_VFD_H__
