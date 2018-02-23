/*  WebOutlet v1.01 by VDG
 *  Remote PowerOutlet WEB control on base ESP8266 chip.
 *  Auto-reset power if there are no answer on pings. 
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  24.05.2017 v1.01 created by VDG
 *  
 *  Main module
 *  
 *  1) PowerOn. Relay ON.
 *  2) If selected AP mode (Factory defaults) - Pinger is off.
 *     Controller stays in AP untill it will be switched to STA with WEB interface. 
 *  3) Trying to get IP. LED flashes 500 on 1000 off mc.
 *     If connection is established - All Ok. Go to .
 *  4) Wait for 60 sec for STA. If WiFi is not connected (or password wrong), controller switches to AP mode for 10 min. 
 *     After 10 min in AP it goes to 3 - try to connect as STA.
 *  5) Pause 10 min (Post Restart Delay).
 *  6) Ping address with TTL 30 sec (Ping TTL).
 *     If connection lost - go to 3.
 *  7) If there is answer - pause 1 min (Ping Interval) and on 6. Led blinks 1000 On 20 off 
 *  8) If there is no answer - try repeat ping (Ping Retries). Led blinks 1000 On 200 off 
 *  9) If there is still no answer - go to next address. Led blinks 1000 On 500 off 
 *  10) If all address are processed and there was no answers - reset power on 15 sec (Restart PowerOff). 
 *     Led blinks 100 On 200 off while in reset mode.
 *     go to 5
 *     
 *  Button: 
 *  2 presses or Long press over 5 sec - go to Power reset
 *  4 presses - toggle power -off
 *  6 presses - toggle ping check mode (only when WiFi is in Client mode)
 *  10 presses - Clear flash and start in AP mode for configuration
 *  
 */
 
#include "main.h"

extern "C" 
{ 
  #include <user_interface.h>
}
 
CGlobalData gD;

//-----------------------------------------------------------------------------

void setup(void) 
{ gD.Start();
}

void loop(void) 
{ gD.Run();
}

//-----------------------------------------------------------------------------
 
void  CGlobalData::Start(void) 
{ 
  Serial.begin( 115200 );
  pinMode( PIN_RELAY, OUTPUT );
  digitalWrite( PIN_RELAY, 1 );

  mBlnk.SetTimes( 0, 500, 1000 );   // Reg WiFi
  mBlnk.SetTimes( 1, 2000, 20 );    // All Ok
  mBlnk.SetTimes( 2, 2000, 200 );   // No PING asw
  mBlnk.SetTimes( 3, 2000, 500 );   // No ADDR asw
  mBlnk.SetTimes( 4, 50,   200 );   // In Power Reset pulse 

  // Read data from eeprom
  if( !VM_FlashRd( 0, &_St, &_Wr-&_St ) )
  { Defaults();
    VM_LedFlash( PIN_LED, 20, 20, 50 ); // quick flashes to signal EEPROM failure
  }

  mWf_ModeCur = mWf_Mode;
  
  mIrda.enableIRIn();             // Start the IR receiver
  wifi_station_set_hostname( mWf_Name );
}

//-----------------------------------------------------------------------------

void  CGlobalData::Run(void) 
{ int   i = mBtn.Get();

  switch( i )
  { case 4: // Power On/Off
      mbPingerOn = 0;
      i = digitalRead( PIN_RELAY );
      digitalWrite( PIN_RELAY, !i );
      break;
      
    case 6: // Power On/Off
      mbPingerOn = !mbPingerOn;
      break;

    case 10: // Factory settings
      Defaults();
      VM_FlashWr( 0, &_St, &_Wr-&_St );
      break;

    default:
      if( (i & 0x40) == 0 ) break; // Not a long press
    case 2:
      RstOutlet();
      mbPingerOn = 1;
      break;
  }

  mBlnk.Tick();

  if( !mfCall ) mfCall = &CGlobalData::fInit;
  else (this->*mfCall)();
  
  mSrv.handleClient();
  if( !mWf_ModeCur ) mDns.processNextRequest();

  // Process IRDA command
  if( !mIrda.decode(&mIrdaRes) ) return;
  
  mIrCommand = mIrdaRes.value & 0xFFFF;
  mIrda.resume(); // Receive the next value 
  for( i=0; i<sizeof(mIR_Commands)/sizeof(mIR_Commands[0]); i++ )
    if( mIrCommand == mIR_Commands[i] )
    { switch( i )
      { case 0: // Relay ON
          mbPingerOn = 0;
          digitalWrite( PIN_RELAY, 1 );
          break;
        case 1: // Realy OFF
          mbPingerOn = 0;
          digitalWrite( PIN_RELAY, 0 );
          break;
        case 2:   // Restart
          RstOutlet();
          break;
        case 3: // ping check ON
          mbPingerOn = 1;
          break;
        case 4: // ping check Off
          mbPingerOn = 0;
          break;
      }
      break;
    }
}

//-----------------------------------------------------------------------------

void  CGlobalData::CheckAP(void)
{ if( !mWf_Mode ) return; // AP mode selected.
  if( mWf_ModeCur ) return; // STA mode is active already.

  // Want STA, but active AP
  if( millis() - mWebSessTm < 2*60*1000L ) mTmAP = millis(); // Some activity in AP/WEB detected. Reset AP->STA interval.
  if( millis() - mTmAP < 10*60*1000L ) return; // STA conn failed. Wait 10 min in AP before reconnecting
  Serial.println( " Trying STA again." );
  mWf_ModeCur = 1;
  mfCall = &CGlobalData::fInit;
}

//-----------------------------------------------------------------------------

void  CGlobalData::fInit(void)
{ 
  Serial.print( "\nWebOutlet v1.01." );

  mBlnkMode = 1;  // All Ok
  mBlnk.Set( 0 ); // WiFi reg
  mDns.stop();
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  delay( 1000 );
  WebInit();
  mTmAP = millis();

  if( mWf_ModeCur )
  { mfCall =  &CGlobalData::fWF_Clnt;
  } else 
  { mfCall =  &CGlobalData::fWF_AP;
  }
}

//-----------------------------------------------------------------------------

void  CGlobalData::fWF_AP(void)
{ // Create access point
  char      Tb[128];
  IPAddress ip(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);  // set gateway to match your wifi network
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your wifi network
  WiFi.config( ip, gateway, subnet );
  sprintf( Tb, "Mode:AP, Name:%s, Pwd:%s, 192.168.1.1/255.255.255.0, IP:", mWfAP_Id, mWfAP_Pwd );
  Serial.print( Tb );
  WiFi.softAP( mWfAP_Id, mWfAP_Pwd );
  ip = WiFi.softAPIP();
  mIP.uL = uint32_t(ip);
  Serial.print( ip );
  mDns.setTTL(300);
  mDns.start( 53, "*", ip );
  mfCall = &CGlobalData::fPingPauseLong;
}

//-----------------------------------------------------------------------------

void  CGlobalData::fWF_Clnt(void)
{ // Create Client
  char      Tb[128];  
  sprintf( Tb, "Mode:STA, Name:%s, Pwd:%s, IP:", mWfCL_Id, mWfCL_Pwd );
  Serial.print( Tb );
  WiFi.mode(WIFI_STA);
  WiFi.begin( mWfCL_Id, mWfCL_Pwd );  
  mfCall = &CGlobalData::fWF_ClntWait;
}

//-----------------------------------------------------------------------------

void  CGlobalData::fWF_ClntWait(void)
{ // Wait for connection
  int i = WiFi.status();

  if( i == WL_CONNECTED )
  { IPAddress ip = WiFi.localIP();
    mIP.uL = uint32_t( ip );
    Serial.print( ip );
  
    mfCall = &CGlobalData::fPingPauseLong;
    return;
  }
  
  if( millis() - mTmAP < 60000 ) return;
  
  if( mWf_Mode ) 
  { mWf_ModeCur = 0;
    Serial.print( "Timeout. Loading AP." );
  }

  mfCall = &CGlobalData::fInit;
}

//-----------------------------------------------------------------------------

void  CGlobalData::fPingPauseLong( void )
{ mTm = millis();
  mfCall = &CGlobalData::fPingPauseLongW;
  mBlnk.Set( mBlnkMode ); // All Ok
}

//-----------------------------------------------------------------------------

void  CGlobalData::fPingPauseLongW( void )
{ if( millis() - mTm < mRstIdle*1000*60L ) 
  { CheckAP();
    return;
  }
  
  if( !mbPingerOn )
  { mBlnkMode = 1;
    mfCall = &CGlobalData::fPingPause;
    return;
  }
  mPingCntr = 0;
  mPingPhase = 0;
  mfCall = &CGlobalData::fPingPauseW;
}

//-----------------------------------------------------------------------------

void  CGlobalData::fPingPause( void )
{ mTm = millis();
  mBlnk.Set( mBlnkMode ); // All Ok
  
  if( WiFi.status() == WL_CONNECTED ) mfCall = &CGlobalData::fPingPauseW;
  else {  mfCall = &CGlobalData::fInit;  
          Serial.println( " WiFi:Connection lost." );
        }
}

//-----------------------------------------------------------------------------

void  CGlobalData::fPingPauseW( void )
{ if( millis() - mTm < mPingInterval*1000 )
  { CheckAP();
    return;
  }
  
  if( !mbPingerOn )
  { mBlnkMode = 1;
    mfCall = &CGlobalData::fPingPause;
    return;
  }
  mPingCntr = 0;
  mPingPhase = 0;
  mfCall = &CGlobalData::fPingProc;
  
  int   i, n;

  for( i=n=0; i<sizeof(mAddr)/sizeof(mAddr[0]); i++ ) if( mAddr[i].uL ) n++;
  if( !n ) mfCall = &CGlobalData::fPingPause; // No TCP addr set => nothing to send.
}

//-----------------------------------------------------------------------------

void  CGlobalData::fPingProc( void )
{ int   i;

  switch( mPingPhase )
  {
    default: // Send new ping
      if( mPingCntr++ >= mPingRetry )
      { // No answer on pings
        if( !mbPingerOn )
        { // Ping control was turned off.
          mBlnkMode = 1;
          mfCall = &CGlobalData::fPingPause;
          break;
        }
        
        if( ++mAddrCntr >= sizeof(mAddr)/sizeof(mAddr[0]) )
        { mBlnkMode = 3;
          mfCall = &CGlobalData::fPowerRst;
          break;
        }
        if( !mAddr[mAddrCntr].uL ) break; // Address not set
        mPingCntr = 0;
      }

      if( ++mPingSeqNo & 0x8000 ) mPingSeqNo = 1;
      mPingPhase = 1; // Wait for data
      mPingMs = millis();

      i = PingSend( mAddr[mAddrCntr].uB, mAddr[mAddrCntr].uS[1], mPingSeqNo );
      if( !i ) 
      {   // Failed to send
          break;
      }
      break;

    case 1: // Wait for answer
      if( millis() - mPingMs < mPingTTL*1000 ) break;
      // No answer received!
      mBlnkMode = mAddrCntr ? 3:2; // All Ok
      mBlnk.Set( mBlnkMode );
      mPingPhase = 0;
      break;

    case 2: // Answer received
      mAddrCntr = 0;
      mPingCntr = 0;
      mPingPhase = 0;      
      if( mAddrCntr ) mBlnkMode = 3;
      else if( mPingCntr > 1 ) mBlnkMode = 2;
           else mBlnkMode = 1;
      mfCall = &CGlobalData::fPingPause;
      break;
  }
}

//-----------------------------------------------------------------------------

void  CGlobalData::fPowerRst( void )
{
  mBlnk.Set( 4 );// Power reset
  mTm = millis();
  digitalWrite( PIN_RELAY, 0 ); // Turn Off Power outlet
  mfCall = &CGlobalData::fPowerRstW;
}

//-----------------------------------------------------------------------------

void  CGlobalData::fPowerRstW( void )
{ if( millis() - mTm < mRstLength*1000 ) return; // Wait for 15 sec
  digitalWrite( PIN_RELAY, 1 ); // Turn Off Power outlet
  mfCall = &CGlobalData::fPingPause;  
}

//-----------------------------------------------------------------------------

