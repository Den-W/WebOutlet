/* Copyright (C) 2012 Kristian Lauszus, TKJ Electronics. All rights reserved.

 */

#ifndef _Vdg_Mod_h_
#define _Vdg_Mod_h_

#define ctl_assert(e) ((void)sizeof(char[1 - 2*!(e)]));

byte 	VM_Crc8( byte crc, byte ch );
bool	VM_FlashRd( short Offset, void *To, short ToSz ); 	// Read data from flash
bool	VM_FlashWr( short Offset, const void *From, short FrSz );	// Write data to flash

void	VM_Led13Flash( byte No, short MsLight, short MsDark );	// Flash BuiltIn LED. ON:OUTPUT-HIGH, OFF:INPUT-LOW
void	VM_LedFlash( byte Pin, byte No, short MsLight, short MsDark ); // Set pin to HIGH for MsLight, then LOW for MsDark

#endif
