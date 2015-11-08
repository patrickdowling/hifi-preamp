// ***************************************************************************
/*
 Project:	Preamp
 Purpose:	Common declarations for preamp project
 */
// ***************************************************************************

#ifndef __PREAMP_H__
#define __PREAMP_H__

#ifdef __cplusplus
extern "C" {
#endif
    
    
#include <inttypes.h>
#include "config.h"
    
    // ---------------------------------------------------------------------------
    /*
     structure to hold preamp input configuration
     */
    // ---------------------------------------------------------------------------
    struct input_parameters
    {
        int8_t	offset;		// gain offset for this input
        int8_t	balance;	// balance settings
    };
    
    
    enum EMenuMode 
    {   eMode_Normal
    ,   eMode_DebugRC5
    ,   eMode_VersionInfo
    };
    
    
    // ---------------------------------------------------------------------------
    /*
     Structure to hold preamp state
     */
    // ---------------------------------------------------------------------------
    struct preamp_state
    {
        // General settings
        uint8_t	volume;
        uint8_t	mute:1;
        uint8_t	active_input:3;
        
        // Display parameters
        uint8_t	luminance:2;
        
        // Per-input settings
        struct input_parameters	inputs[ ACTIVE_INPUTS ];
        
        // Update flags
        union
        {
            uint8_t	raw;
            struct
            {
                uint8_t	update_volume:1;
                uint8_t	update_input:1;
                uint8_t	update_mute:1;
                uint8_t update_display:1;
            } bits;
        } flags;
        
        uint8_t show_volume:2;
        uint8_t hide_volume:1;
        uint8_t show_input:1;
        uint8_t force_clear:1;
        
        uint8_t mode:2;
        
        unsigned long luminance_time;
        unsigned long volume_time;
        unsigned long input_time;
        unsigned long last_mute_time;        
    };
    
    // ---------------------------------------------------------------------------
    //
    // Basic input port defines (assume all input on same port)
    //
    // ---------------------------------------------------------------------------
#define INPUT_PORT            PIND
#define RC5_PIN		      _BV(PIND3)
#define ENCODER_PIN_A 	      _BV(PIND4)
#define ENCODER_PIN_B	      _BV(PIND5)
#define KEY_SELECT 	      _BV(PIND6)
#define KEY_MASK	      ( KEY_SELECT )
    
    // ---------------------------------------------------------------------------
    //
    // SPI
    //
    // ---------------------------------------------------------------------------
#define SPI_SCK		_BV(PINB5)
#define SPI_MISO	_BV(PINB4)
#define SPI_MOSI	_BV(PINB3)
#define SPI_SS		_BV(PINB2)
    
#define PRE_PORT        PORTB
#define PRE_CS		SPI_SS
#define PRE_MUTE	_BV(PINB1)
    
#define VFD_MB		_BV(PINB0)
#define VFD_SS		_BV(PIND7)
    
#define VFD_PIN          PINB
#define VFD_PORT         PORTD
    
    
    int preamp_main( void );
    
#ifdef __cplusplus
}
#endif

#endif // __PREAMP_H__
