// ***************************************************************************
/*
 Project:	Preamp
 Purpose:	Declaration of configurable values
 */
// ***************************************************************************

#ifndef __PREAMP_CONFIG_H__
#define __PREAMP_CONFIG_H__

#define DEFAULT_VOLUME      0x70	// -40.0dB
#define DEFAULT_INPUT       0		// Input 1 (CD)
#define DEFAULT_MUTE_STATE	1       // mute on

#define ACTIVE_INPUTS	6

#define SHOW_VOLUME_TIME    1500
#define SHOW_INPUT_TIME     2000
#define LUMINANCE_TIME      15000
#define LUMINANCE_FADE_STEP 150

#define MUTE_DEBOUCE_TIME   1000

// ---------------------------------------------------------------------------
//
// CPU definitions
//
// ---------------------------------------------------------------------------
#ifndef F_CPU
# define F_CPU 	16e6
#endif

#define XTAL	F_CPU

#endif // __PREAMP_CONFIG_H__
