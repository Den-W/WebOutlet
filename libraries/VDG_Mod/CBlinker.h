/* Copyright (C) 2017. All rights reserved.

 */

#ifndef _CBlinker_h_
#define _CBlinker_h_

#ifndef BLINK_MODES
 #define BLINK_MODES	8
#endif

class CBlinker
{public:
  uint32_t	mTime;
  byte		mPin;
  byte		mMode;
  byte  	mBlinkOn[BLINK_MODES];
  byte  	mBlinkOff[BLINK_MODES];

  CBlinker( byte BlinkPin = 13 )
  { mMode = 0xFF;
    mPin = BlinkPin;
    pinMode( mPin, OUTPUT );     // Set OnBoad LED as an output  
//ctl_assert( BLINK_MODES	!= 8 ) 

#if BLINK_MODES > 0
    mBlinkOn[0] = 1;   mBlinkOff[0] = 200; // Rare, short blink
#endif
#if BLINK_MODES >= 1
    mBlinkOn[1] = 1;   mBlinkOff[1] = 3;   // Quick
#endif
#if BLINK_MODES >= 2
    mBlinkOn[2] = 5;   mBlinkOff[2] = 20; 
#endif
#if BLINK_MODES >= 3
    mBlinkOn[3] = 200; mBlinkOff[3] = 2; // Rare, Inverse short
#endif
  }

  int  Set( byte Mode = 0xFF )
  {  if( Mode < BLINK_MODES )
     { mTime = 0;
       mMode = Mode;
       Tick();
       return 1;
     }
     mMode = 0xFF;
     digitalWrite( mPin, LOW );
     return 0;
  }

  void  SetTimes( byte Mode, short TmOnMs, short TmOffMs )
  { if( Mode < BLINK_MODES )
    { TmOnMs /= 25;
      TmOffMs /= 25;
      mBlinkOn[Mode] = TmOnMs ? TmOnMs:1;
      mBlinkOff[Mode] = TmOffMs ? TmOffMs:1;
    }
  }

  void  Tick( void ) 
  { if( mMode & 0x80 ) return;
    if( mMode & 0x40 ) 
    {   if( millis()-mTime < mBlinkOff[mMode&0xF]*25L ) return;
	mMode &= ~0x40;
	digitalWrite( mPin, LOW );
    } else 
    {  if( millis()-mTime < mBlinkOn[mMode&0xF]*25L ) return;
       mMode |= 0x40;
       digitalWrite( mPin, HIGH );
    }
    mTime = millis();
  }
};

#endif
