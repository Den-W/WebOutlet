/* Copyright (C) 2017. All rights reserved.
 * 
 * Read keypresses. 
 * Get() returns number of detected keypresses. Get() & 0x40 => there was long keypress
 */

#include <Arduino.h>
#include "VDG_Mod.h"

#ifndef _CKey_h_
#define _CKey_h_

#ifndef CK_TmPRESS
 #define CK_TmPRESS	50 	// Keyress shorter than 100ms ignored
#endif

#ifndef CK_TmWAIT
 #define CK_TmWAIT	1000 	// Wait 1000ms for another keyress when previous ended
#endif

#ifndef CK_TmLONG
 #define CK_TmLONG	1500	// Keyress longer than 1500ms is a LongPress
#endif

class CKey
{public:
  uint32_t	mTime;
  byte		mPin;
  byte		mLevel;
  byte		mCntr; 		// 0x80 - IsKeypress, 0x40 - IsLp, 0x3F - Press counter
  
  CKey( byte PinButton = 13, byte ActiveLevel = LOW )
  { mCntr = 0;
    mTime = 0;
    mPin = PinButton;
    mLevel = ActiveLevel;
  }

  byte  Get( void ) 	//! 0x40 - IsLp, 0x3F - Press counter
  { byte	rc = 0;
    uint32_t	ms = millis() - mTime;

    if( digitalRead( mPin ) == mLevel ) // Button pressed?
    { 	mCntr |= 0x80;
	return 0;
    }

    while( 1 )
    {	// Button released
    	if( mTime == 0 ) break;

	if( mCntr & 0x80 )
    	{ mCntr &= 0x7F;
	  if( ms < CK_TmPRESS ) break; // ignore short presses

	  mCntr++;
	  if( ms >= CK_TmLONG ) mCntr |= 0x40;
  	  break;       
    	}
    	
    	if( mCntr == 0 ) break;
	if( ms < CK_TmWAIT ) return 0; // Waiting for another keypress

        rc = mCntr;
  	mCntr = 0;
	break;     
    }
    
    mTime = millis();
    return rc;
  }

};

#endif
