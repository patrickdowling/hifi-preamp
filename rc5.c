// ***************************************************************************
/*
	Purpose:	Implementation for rc5.h
*/
// ***************************************************************************

#include "rc5.h"

void
rc5_packet_decode( uint16_t rc5value, struct rc5_packet* packet )
{
    packet->valid   = rc5value != 0 ? 1 : 0;
    packet->toggle  = ( rc5value >> 11 & 0x0001 ) ? 1 : 0;
    packet->address = (uint8_t)( rc5value >> 6 & 0x001F );
    packet->code    = (uint8_t)( rc5value & 0x003F ) | ( ~rc5value >> 6 & 0x0040 ); // extended rc5 uses inverted start bit 2 as 7th bit of command
}
