// ***************************************************************************
// ****************************************************************************
//
// Purpose:	Preamp main entry point...
//
// ****************************************************************************
// ***************************************************************************

#include "config.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>

#include "encoder.h"
#include "icons.h"
#include "preamp.h"
#include "rc5.h"
#include "spi.h"
#include "vfd.h"

// ---------------------------------------------------------------------------
//
// State variables
//
// ---------------------------------------------------------------------------
volatile struct encoder_state	g_EncoderState 	  = { DEFAULT_VOLUME, 0, 0 };
volatile struct rc5_state       g_RC5State        = { 0, 0, 0, 0 };
volatile uint8_t		g_KeyState      = 0;

char _LABEL0[] /*PROGMEM*/ = "CD        ";
char _LABEL1[] /*PROGMEM*/ = "SQUEEZEBOX";
char _LABEL2[] /*PROGMEM*/ = "IPOD DOCK ";
char _LABEL3[] /*PROGMEM*/ = "INPUT 4   ";
char _LABEL4[] /*PROGMEM*/ = "INPUT 5   ";
char _LABEL5[] /*PROGMEM*/ = "INPUT 6   ";

const char* INPUT_LABELS[] /*PROGMEM*/ =
{   _LABEL0
,   _LABEL1
,   _LABEL2
,   _LABEL3
,   _LABEL4
,   _LABEL5
};

const prog_char _FIRMWARE[] PROGMEM = "FIRMWARE: "__DATE__ " " __TIME__;
const prog_char _HARDWARE[] PROGMEM = "HARDWARE: hifiakademie + arduino";

// default state 
struct preamp_state g_PreampState;

#define TIMER_COUNT  -2

char strbuf[ 32 ] = "";

// ---------------------------------------------------------------------------
//
// Local functions
//
// ---------------------------------------------------------------------------
static void
set_preamp_state( const struct preamp_state * state );

static void 
display_rc5( const struct rc5_packet* packet, const uint16_t rc5value );

static void
display_normal( struct  preamp_state* preampState, unsigned long now );

static void
display_version( void );

static void
update_luminance( struct preamp_state* pState, unsigned long now );

static void
handle_rc5( const struct rc5_packet* packet, struct preamp_state* preampState, unsigned long now );

// Arduino SDK function
extern unsigned long millis( void );

/*****************************************************************************
 *
 *  Main setup (arduino)
 */
void setup( void )
{   
    // Setup ports this is a bit tricky because of stupidly my layouted board
    
    DDRC  = 0xFF;    // Port C = Arduino A0-A7 currently unused, configure as output
    PORTC = 0x00;
    
    // SPI setup
    PORTB = 0xFF;
    DDRB  = ~SPI_MISO; // ( SPI_SCK | SPI_MOSI | SPI_SS );
    
    // preamp control pins
    PORTB &= ~PRE_MUTE;
    PORTB |= PRE_CS;
    
    // VFD control pins
    DDRB &= ~VFD_MB;
    PORTB &= ~VFD_MB;
    
    DDRD   = 0xFF;
    PORTD  = 0x0;
    
    // Input pins (external pullups)
    // D0/D1 used by USB
    DDRD   &= ~( RC5_PIN | ENCODER_PIN_A | ENCODER_PIN_B | KEY_SELECT );
    PORTD  &= ~( RC5_PIN | ENCODER_PIN_A | ENCODER_PIN_B | KEY_SELECT );
    
	//
	// Initialization
	//
	spi_init();
	vfd_init();
    
    // default state 
    g_PreampState.volume		= DEFAULT_VOLUME;
    g_PreampState.mute 			= DEFAULT_MUTE_STATE;
    g_PreampState.active_input 	= DEFAULT_INPUT;
    g_PreampState.luminance		= 0;
    g_PreampState.flags.raw     = 0xFF;
    
    g_PreampState.luminance_time    = 0;
    g_PreampState.volume_time       = 0;
    g_PreampState.input_time        = 0;
    g_PreampState.last_mute_time    = 0;

    g_PreampState.show_volume = 0;
    g_PreampState.hide_volume = 0;
    g_PreampState.show_input  = 1;
    g_PreampState.force_clear = 0;
    
    g_PreampState.mode = eMode_Normal;   
    
	set_preamp_state( &g_PreampState );
    
	//
	// Setup timer 1
	//
    cli();       
    TCCR1A = 0x00;    // disconnect from external pins
    TCCR1B = ( 1 << CS12 );  // setup timer interval (CLK/256)
    // TCCR1B = ( 1 << CS12 ) | ( 1 << CS10 );  // setup timer interval (CLK/1024)
    TIMSK1 = ( 1 << TOIE1 );  // enable overflow interrupt
    TCNT1 = TIMER_COUNT; // duty cycle
	sei();
    
	set_sleep_mode( SLEEP_MODE_IDLE );
    
}

/*****************************************************************************
 *
 *  Main entry point (arduino)
 */
void loop( void )
{   
	uint8_t keyValue                = -1;   
    
	//
	// Main loop
	//
	for(;;)
	{
        const unsigned long now = millis();
        struct preamp_state* preampState = &g_PreampState;

        preampState->force_clear = 0;
        
        //
        // Handle last stored RC5 command
        //
		cli();      // non-atomic access to uin16_t variables, need to lock interrupts
		const uint16_t rc5value = g_RC5State.rc5_data;
		g_RC5State.rc5_data = 0;
		sei();
        struct rc5_packet   rc5packet;
        rc5_packet_decode( rc5value, &rc5packet );
        
		if ( rc5packet.valid )
            handle_rc5( &rc5packet, preampState, now );
        
        //
        // Handle encoder
        //
		const encoder_value_t encoderValue = g_EncoderState.value;
        
		if ( preampState->volume != ENCODER_CONV( encoderValue ) )
		{
			preampState->volume = ENCODER_CONV( encoderValue );
			preampState->flags.bits.update_volume = 1;
            preampState->volume_time = now;
            preampState->show_volume = 1;
            preampState->hide_volume = 0;
		}
        
        //
        // Handle hardkeys
        //
		const uint8_t newKeyValue = g_KeyState;
		if ( newKeyValue != keyValue )
		{
			// toggle mute state on key down
			if ( ( newKeyValue & KEY_SELECT ) && !( keyValue & KEY_SELECT ) )
			{
                if ( eMode_Normal == preampState->mode )
                {
                    preampState->mute = 1 - preampState->mute;
                    preampState->flags.bits.update_mute = 1;
                }
                else
                {
                    preampState->mode = eMode_Normal;
                    preampState->force_clear = 1;
                }
			}
            
            keyValue = newKeyValue;
		}
        
        // Clear display if required
        if ( preampState->force_clear )
            vfd_cmd( VFD_CLEAR );
        
        if ( eMode_Normal == preampState->mode )
        {
            // Update actual preamp status
            if ( preampState->flags.raw )
            {
                set_preamp_state( preampState );            
                preampState->luminance	= 0;
                preampState->luminance_time = now;
            }

            // Update of display contents
            display_normal( preampState, now );
             
            // Update luminance of display
            update_luminance( preampState, now );
            
            // Reset flags
            preampState->flags.raw = 0;
        }
        else if ( eMode_DebugRC5 == preampState->mode )
            display_rc5( &rc5packet, rc5value );
        else if ( eMode_VersionInfo == preampState->mode && preampState->force_clear )
            display_version();
        
		sleep_mode();
	}
}

// ****************************************************************************
// ****************************************************************************
static void
display_normal( struct preamp_state* preampState, unsigned long now )
{
    // 
    // Icon area (left)
    //
    if ( preampState->flags.bits.update_mute || preampState->flags.bits.update_input || preampState->force_clear )
    {
        const struct icon_descriptor* icon = preampState->mute ? getIconPtr( ICON_MUTE ) : getIconPtr( ICON_INPUT1 + preampState->active_input );
        vfd_draw( ICON_AREA_X, 0, ICON_AREA_W, VFD_RES_Y, 'C' );
        vfd_draw_icon( ICON_AREA_X + ( ICON_AREA_W - icon->width ) / 2, 0, icon );
    }
    
    //
    // Volume area (right)
    //
    if ( preampState->flags.bits.update_volume || preampState->force_clear )
    {
        vfd_cmd_param( VFD_SETFONT, FONT_10x14 );
        vfd_cmd_param( VFD_SETFONT, '1' );
        
        // Gain (dB) = 31.5 - [0.5 x (255 - N)]
        const int16_t dB = 315 - ( ( (ENCODER_MAX - preampState->volume ) * 10 ) / 2 );
        sprintf_P( strbuf, PSTR( "%+3d.%01ddB " ), dB / 10, dB < 0 ? -dB % 10 : dB % 10 );
        vfd_string_at( VOLUME_X, VOLUME_Y, strbuf );
        
        vfd_cmd_param( VFD_SETFONT, '2' );
    }
    
    //
    // Middle area is either input or volume bar
    //
    if ( preampState->show_volume || preampState->hide_volume || preampState->force_clear )
    {
        if ( preampState->hide_volume || ( now - preampState->volume_time ) > SHOW_VOLUME_TIME )
        {
            preampState->hide_volume = 0;
            preampState->show_volume = 0;
            preampState->flags.bits.update_input = 1;
        }
        if ( preampState->show_volume < 2 )
        {
            vfd_draw( MIDDLE_AREA_X, 0, MIDDLE_AREA_W, VFD_RES_Y, VFD_DRAW_CLEAR );
        }
        
        if ( preampState->show_volume == 1 )
        {
            vfd_draw( VOLUME_BAR_X + 2, 3, VOLUME_BAR_W - 2, 11, VFD_DRAW_OUTLINE );
            vfd_draw( VOLUME_BAR_X + 4, 5, (VOLUME_BAR_W - 5) * preampState->volume / 255, 7, VFD_DRAW_FILL );
            ++preampState->show_volume;
        }
    }
    
    if ( preampState->show_input && ( now - preampState->input_time ) > SHOW_INPUT_TIME )
    {
        preampState->show_input = 0;
        preampState->flags.bits.update_input = 1;
    }
    
    if ( ( preampState->flags.bits.update_input || preampState->force_clear ) && !preampState->show_volume )
    {
        vfd_draw( MIDDLE_AREA_X, 0, MIDDLE_AREA_W, VFD_RES_Y, VFD_DRAW_CLEAR ); // brute force
        
        if ( preampState->show_input )
        {
            vfd_draw( INPUT_FIELD_X, 0, ACTIVE_INPUTS * INPUT_CHAR_W, VFD_RES_Y, VFD_DRAW_CLEAR );
            vfd_cmd_param( VFD_SETFONT, FONT_5x7 );
            
            for ( int i = 0; i < ACTIVE_INPUTS; ++i )
            {
                const uint8_t x = INPUT_TEXT_X + ( 2 + ( i * 10 ) ) % ( ACTIVE_INPUTS / 2 * INPUT_CHAR_W );
                const uint8_t y = i < ACTIVE_INPUTS / 2 ? 0 : VFD_RES_Y / 2;
                
                const char numberLabel[] = { '1' + i, 0 };
                vfd_string_at( x + INPUT_CHAR_X_OFFSET, y + INPUT_CHAR_Y_OFFSET, numberLabel );
                if ( i ==  preampState->active_input )
                    vfd_draw( x, y, INPUT_CHAR_W, VFD_RES_Y / 2, VFD_DRAW_INVERT );
            }
        }
        
        vfd_cmd_param( VFD_SETFONT, FONT_10x14 );
        vfd_string_at( preampState->show_input ? INPUT_LABEL_X : INPUT_TEXT_X, 15, INPUT_LABELS[  preampState->active_input ] );
    }
}

// ****************************************************************************
// ****************************************************************************
static void
display_version( void )
{
    vfd_cmd_param( VFD_SETFONT, FONT_5x7 );
    vfd_string_at_p( 0, 7, _FIRMWARE );
    vfd_string_at_p( 0, 15, _HARDWARE );
}


// ****************************************************************************
// ****************************************************************************
static void
handle_rc5( const struct rc5_packet* rc5packet, struct preamp_state* preampState, unsigned long now )
{
    int8_t	volume_delta = 0;
    switch( rc5packet->code )
    {
        case IR_KEY_1:
        case IR_KEY_2:
        case IR_KEY_3:
        case IR_KEY_4:
        case IR_KEY_5:
        case IR_KEY_6:
            if ( eMode_Normal == preampState->mode )
            {
                preampState->flags.bits.update_input = ( preampState->active_input != rc5packet->code - 1 ) ? 1 : 0;
                preampState->active_input = rc5packet->code - 1;
                preampState->input_time = now;
                preampState->show_input = 1;
            }
            break;
            
        case IR_KEY_UP:
            volume_delta += 1;
            break;
            
        case IR_KEY_DOWN:
            volume_delta -= 1;
            break;
            
        case IR_KEY_MUTE:
            if (eMode_Normal == preampState->mode && now - preampState->last_mute_time > MUTE_DEBOUCE_TIME )
            {
                preampState->mute = 1 - preampState->mute;
                preampState->flags.bits.update_mute = 1;
                preampState->last_mute_time = now;
            }
            break;
            
        case IR_KEY_GREEN:
            if ( eMode_VersionInfo != preampState->mode )
            {
                preampState->force_clear = 1;
                preampState->mode = eMode_VersionInfo;
            }
            break;
            
        case IR_KEY_BLUE:
            if ( eMode_DebugRC5 != preampState->mode )
            {
                preampState->force_clear = 1;
                preampState->mode = eMode_DebugRC5;
            }
            break;
            
        case IR_KEY_OK:
            if ( eMode_VersionInfo == preampState->mode )
            {
                preampState->mode = eMode_Normal;
                preampState->force_clear = 1;
            }
            break;
    }
    
    if ( volume_delta && eMode_Normal == preampState->mode )
    {
        cli();
        if ( volume_delta > 0 && g_EncoderState.value < ENCODER_MAX )
            g_EncoderState.value += 1;
        else
            if ( volume_delta < 0 && g_EncoderState.value > 0 )
                g_EncoderState.value -= 1;
        sei();
    }
}

// ****************************************************************************
// ****************************************************************************
SIGNAL (SIG_OVERFLOW1)
{
    const uint8_t input = INPUT_PORT;
    
    uint16_t tmp = g_RC5State.rc5_tmp;				// for faster access
    
    if( ++g_RC5State.rc5_time > PULSE_MAX ){			// count pulse time
        if( !(tmp & 0x4000) && tmp & 0x2000 )	// only if 14 bits received
            g_RC5State.rc5_data = tmp;
        tmp = 0;
    }
    
    if( (g_RC5State.rc5_bit ^ input) & RC5_PIN ){		// change detect
        g_RC5State.rc5_bit = ~g_RC5State.rc5_bit;				// 0x00 -> 0xFF -> 0x00
        
        if( g_RC5State.rc5_time < PULSE_MIN )			// to short
            tmp = 0;
        
        if( !tmp || g_RC5State.rc5_time > PULSE_1_2 ){		// start or long pulse time
            if( !(tmp & 0x4000) )			// not to many bits
                tmp <<= 1;				// shift
            if( !(g_RC5State.rc5_bit & RC5_PIN) )		// inverted bit
                tmp |= 1;				// insert new bit
            g_RC5State.rc5_time = 0;				// count next pulse time
        }
    }
    
    g_RC5State.rc5_tmp = tmp;
    
	//
	// Handle keys
	//
	static uint8_t 	key_state_count0 = 0;
	static uint8_t	key_state_count1 = 0;
    
	uint8_t	changed_keys = g_KeyState ^ ~( input & KEY_MASK );
	key_state_count0 = ~key_state_count0;
	key_state_count1 = key_state_count0 ^ ( key_state_count1 & changed_keys );
	changed_keys &= key_state_count0 & key_state_count1;    
	g_KeyState ^= changed_keys;
    
	//
	// Handle rotary encoder
	//
	volatile struct encoder_state * pState = &g_EncoderState;
    
	// Get current status bit for encoder pins from port
	const uint8_t	newStatus = ( input & ( ENCODER_PIN_A | ENCODER_PIN_B ) );
    
	// Test if one of the pins has changed
	if ( ( newStatus ^ pState->step ) == ( ENCODER_PIN_A | ENCODER_PIN_B ) )
	{
		// Status has changed; test for direction
		if ( ( newStatus ^ pState->status ) == ENCODER_PIN_A )
		{
			// Rotate right
      		if ( pState->value < ENCODER_MAX ) 
				pState->value += 1;
		}
		else
		{
			// Rotate left
      		if ( pState->value > 0 ) 
				pState->value -= 1;
		}        
		pState->step = newStatus;
	}
    
	pState->status = newStatus;
    
    //
    // Reset timer
    //
    TCNT1 = TIMER_COUNT;
}

// ****************************************************************************
// ****************************************************************************
static void
set_preamp_state( const struct preamp_state * state )
{
	PRE_PORT &= ~PRE_CS;	// CS low. PGA2311 & latch active
	
	// Push 0 into latch (2x since latch forwards to PGA and we need to init l&r channels)
	SPIOUT( 0 );
	SPIOUT( 0 );
    
	// Push volume
	SPIOUT( state->volume );
	SPIOUT( state->volume );
    
	// Push relay setting
	SPIOUT( 0x1 << state->active_input );
    
	PRE_PORT |= PRE_CS;	// CS high, data updated
    
	if ( state->mute )
		PRE_PORT &= ~PRE_MUTE;	// Low: mute on
	else
		PRE_PORT |= PRE_MUTE;	// high: mute off
}

// ****************************************************************************
// ****************************************************************************
static void
display_rc5( const struct rc5_packet* rc5packet, const uint16_t rc5value )
{
    if ( !rc5packet->valid )
        return;
    
    vfd_cmd_param( VFD_SETFONT, FONT_5x7 );
    sprintf_P( strbuf, PSTR("%.4x-%.4x-%.1d"), rc5packet->address, rc5packet->code, rc5packet->toggle );
    vfd_string_at( 0, 15, strbuf );

    for ( int i = 0; i < 16; ++i )
        strbuf[i] = ( 0x8000 >> i & rc5value ) ? '1' : '0';
    strbuf[16] = 0;
    
    vfd_string_at( 140, 15, strbuf );
}

// ****************************************************************************
// ****************************************************************************
static void
update_luminance( struct preamp_state* preampState, unsigned long now )
{
    if ( preampState->luminance < 0x03 )
    {
        const unsigned long threshold = preampState->luminance ? LUMINANCE_FADE_STEP : LUMINANCE_TIME;
        if ( ( now - preampState->luminance_time ) > threshold )
        {
            ++preampState->luminance;
            preampState->luminance_time = now;
        }
    }
    
    static uint8_t last_luminace = 0xFF;
    if ( last_luminace != preampState->luminance )
        vfd_cmd_param( VFD_SETLUM, preampState->luminance );
    last_luminace = preampState->luminance;
}
