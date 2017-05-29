/* Copyright (C) 2017 Vidyakin Denis Georgievich. All rights reserved.
 *
 * This software may be distributed and modified freely.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "VDG_Mod.h"

bool	VM_FlashRd( short Offset, void *To, short ToSz )
{ byte	*pb = (byte*)To, b = 0;

  if( Offset+ToSz > 1024 ) return false;

  EEPROM.begin( 
#ifdef ESP_H
	ToSz + 1 
#endif
	      );
  
  for( ; ToSz>0; Offset++,ToSz-- )
  { *pb = EEPROM.read( Offset );
    b = VM_Crc8( b, *pb++ );
     yield();
  }
  
  ToSz = EEPROM.read( Offset ) & 0xFF;
  EEPROM.end();
  return b == ToSz;
}

//-----------------------------------------------------------------------------

bool	VM_FlashWr( short Offset, const void *From, short FrSz )
{  byte	*pb = (byte*)From, b = 0;

  if( Offset+FrSz > 1024 ) return false;

  EEPROM.begin( 
#ifdef ESP_H
	FrSz + 1 
#endif
	);
  
  for( ; FrSz>0; Offset++,FrSz-- )
  { EEPROM.write( Offset, *pb );
    b = VM_Crc8( b, *pb++ );
    yield();
  }
  EEPROM.write( Offset, b );
#ifdef ESP_H
  EEPROM.commit();
#endif
  EEPROM.end();
  return true;
}


//-----------------------------------------------------------------------------

byte VM_Crc8( byte crc, byte ch ) 
{  
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = crc ^ ch;
      crc >>= 1;
      if (mix & 0x01 ) crc ^= 0x8C;
      ch >>= 1;
  }
  return crc;
}
