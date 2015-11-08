// ***************************************************************************
/*
	Project:	preamp
	Purpose:	Define SPI helper functions
*/
// ***************************************************************************

#ifndef __PREAMP_SPI_H__
#define __PREAMP_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif
    

#define SPIOUT( x ) \
	SPDR = x; while(!(SPSR & (1<<SPIF)));

void spi_init( void );

    
#ifdef __cplusplus
}
#endif

#endif // __PREAMP_SPI_H__
