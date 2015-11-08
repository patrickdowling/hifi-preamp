// ***************************************************************************
/*
	Purpose:	Implementation for spi.h
*/
// ***************************************************************************

#include <avr/io.h>
#include "preamp.h"

void
spi_init( void )
{
//	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);	/* Enable SPI, Master, set clock rate fck/16 */ 
  SPCR = ((1<<SPE)|(1<<MSTR));	/* Enable SPI, Master, set clock rate fck/4 */ 
  //SPSR |= _BV(SPI2X);				// Double speed, clock rate = SYSCLK/2
}
