/* Copyright (C) 2017 Vidyakin Denis Georgievich. All rights reserved.
 *
 * This software may be distributed and modified freely.
 */

#include <Arduino.h>
#include "VDG_Mod.h"

void	VM_LedFlash( byte Pin, byte No, short MsLight, short MsDark )
{  pinMode( Pin, OUTPUT );
   for( int i=0; i<No; i++ )
   { digitalWrite( Pin, HIGH );
     delay( MsLight );
     digitalWrite( Pin, LOW );
     delay( MsDark );
   }
}

void	VM_Led13Flash( byte No, short MsLight, short MsDark )
{  for( int i=0; i<No; i++ )
   { pinMode( LED_BUILTIN, OUTPUT );
     digitalWrite( LED_BUILTIN, HIGH );
     delay( MsLight );
     pinMode( LED_BUILTIN, INPUT );
     digitalWrite( LED_BUILTIN, LOW );
     delay( MsDark );
   }
}