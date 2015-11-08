// ***************************************************************************
/*
	Purpose:	Implemenation of vfd.h
*/
// ***************************************************************************
#include "vfd.h"
#include "spi.h"
#include "preamp.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

void
vfd_cmd( const uint8_t cmd )
{
	VFDOUT( 0x0F );
	VFDOUT( cmd );
}

void
vfd_cmd_param( const uint8_t cmd, const uint8_t param )
{
	VFDOUT( 0x0F );
	VFDOUT( cmd );
	VFDOUT( param );
}

void
vfd_string( const char * str )
{
	while ( *str )
	{
		VFDOUT( *str );
		++str;
	}
}

void
vfd_string_p( const prog_char str[] )
{
    char b;
    
	while ( 0 != ( b = pgm_read_byte( *str++ ) ) )
	{
		VFDOUT( b );
	}
}

void 
vfd_string_at( const uint16_t x, const uint8_t y, const char * str )
{
	vfd_cmd( 0xF0 );
	VFDCOORD( x, y );
	vfd_string( str );
}

void
vfd_string_at_p( const uint16_t x, const uint8_t y, const prog_char str[] )
{
    vfd_cmd( 0xF0 );
    VFDCOORD( x, y );
    vfd_string_p( str );
}


void
vfd_data( const uint8_t * data, size_t len )
{
	while ( len-- )
	{
		VFDOUT( *data );
		++data;
	}
}

void 
vfd_data_p( const uint8_t * data, size_t len )
{
	while ( len-- )
	{
		VFDOUT( pgm_read_byte( *data ) );
		++data;
	}
}


void
vfd_draw( const uint16_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint8_t cmd )
{
	VFDOUT( 0x0F );
	VFDOUT( 0xF1 );
	VFDCOORD( x, y );
	VFDCOORD( ( x + w - 1), ( y + h - 1 ) );
	VFDOUT( cmd );
}

void
vfd_set_cursor( const uint8_t x, const uint8_t y )
{
	uint8_t	cmd = ( y == 0 ) ? 0x80 : 0xC0;
	cmd += x;

	vfd_cmd( cmd );
}

void
vfd_init()
{
	// Assume spi_init already called
    
	VFD_PORT |= VFD_SS;		// SS high (VFD not selected)    
	vfd_cmd( VFD_CLEAR );
	vfd_cmd( VFD_HOME );
}

void
vfd_draw_icon( const uint16_t x, const uint8_t y, const struct icon_descriptor* pIcon )
{
    if ( pIcon && pIcon->data )
    {
        vfd_draw( x, y, pIcon->width, pIcon->height, 'h' );
        vfd_data( pIcon->data , pIcon->width / 8 * pIcon->height );
    }
}
