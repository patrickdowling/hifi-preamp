// ***************************************************************************
/*
	Project:	Preamp
	Purpose:	Declaration of icons
*/
// ***************************************************************************
#ifndef __PREAMP_ICONS_H__
#define __PREAMP_ICONS_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include <inttypes.h>

enum eICON_TYPE
{   ICON_MUTE
, 	ICON_MUTEOFF
,	ICON_INPUT1
,	ICON_INPUT2
,	ICON_INPUT3
,	ICON_INPUT4
,	ICON_INPUT5
,	ICON_INPUT6
,   ICON_COUNT
};

struct icon_descriptor
{
    const uint8_t *   data;
    uint8_t     width;
    uint8_t     height;
};


const struct icon_descriptor*	getIconPtr( enum eICON_TYPE type );

    
#ifdef __cplusplus
}
#endif
        
#endif // _PREAMP_ICONS_H__
