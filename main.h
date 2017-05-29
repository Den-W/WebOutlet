/*  WebOutlet v1.01 by VDG
 *  Remote PowerOutlet WEB control on base ESP8266 chip.
 *  Auto-reset power if there are no answer on pings. 
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  24.05.2017 v1.01 created by VDG
 *  
 */
 
#include <Arduino.h>
#include <stdio.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <DNSServer.h>

#include <Vdg_Mod.h>
#include <CKey.h>
#include <CWebVar.h>
#include <CBlinker.h>


/* Arduino PIN Dxx <-> ESP8266 GPIOxx
static const uint8_t D0   = 3;
static const uint8_t D1   = 1;
static const uint8_t D2   = 16;
static const uint8_t D3   = 5;
static const uint8_t D4   = 4;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 0;
static const uint8_t D9   = 2;
static const uint8_t D10  = 15;
*/

// PIN assignment. 
#define PIN_BUTTON  D3  // I GPIO0  - Button
#define PIN_LED     D4  // O GPIO2  - OnBoard LED
#define PIN_IRDA    D5  // I GPIO14 - IRDA receiver
#define PIN_RELAY   D6  // O GPIO12 - Power relay

typedef union
{ uint32_t  uL;
  uint16_t  uS[2];
  byte      uB[4];
} uByteLong;

class CGlobalData : public CWebVar
{
    typedef void (CGlobalData::*pFUNC)( void );    

    pFUNC     mfCall;
    pFUNC     mfCallPrev;

    void  fInit( void );
    void  fWF_AP( void );
    void  fWF_Clnt( void );
    void  fWF_ClntWait(void);
    void  fPingPause( void );
    void  fPingPauseW( void );
    void  fPingPauseLong( void );
    void  fPingPauseLongW( void );
    void  fPingProc( void );
    void  fPowerRst( void );
    void  fPowerRstW( void );
    
  public:
    
    byte    _St;
    byte      mWf_Mode;         // 0: AccessPoint, 1:Client    
    char      mWfAP_Id[16];     // AP WiFi SSID
    char      mWfAP_Pwd[16];    // AP WiFi password    
    char      mWfCL_Id[16];     // Client WiFi SSID
    char      mWfCL_Pwd[16];    // Client WiFi password
    char      mWf_Name[16];     // Device hostname   
    char      mWf_Login[16];    // Admin login
    char      mWf_Pwd[32];      // Admin password

    byte      mRstIdle;         // Pause after Power Off, min
    byte      mPingTTL;         // MAX Ping answer time 
    byte      mPingRetry;       // Retries before 'NoANSWER'
    byte      mbPingerOn;       // Ping control enabled
    uByteLong mAddr[4];         // IP adresses
    uint16_t  mPingInterval;    // Pause after successful Ping, Sec
    uint16_t  mRstLength;       // Power Off seconds
    uint16_t  mIR_Commands[5];  // IR commands. 0:On, 1:Off, 2:Rst, 3:PingOn, 4:PingOff );
    byte    _Wr;                // Data between _St and _Wr stored in flash memory. _Wr == crc8 of the block

    byte      mWf_ModeCur;      // Current WiFi mode
    byte      mPingCntr;        // Ping retries   
    byte      mAddrCntr;        // Curent address
    byte      mBlnkMode;        // How LED works
    
    uint16_t  mIrCommand;       // Last IR command.
    uint16_t  mPingSeqNo;       // 0000-7fff
    uint16_t  mPingPhase;       // 0:Idle, 1:ExpectAsw
    uint32_t  mTm;              // time for intervals
    uint32_t  mTmAP;            // Start AP mode    
    uint32_t  mPingMs;          // ping length

    uint32_t  mWebSessTm;       // Last WEB session access
    char      mWebSessId[14];   // Cookie for login trace
    
    byte    _En;

    CKey              mBtn;
    CBlinker          mBlnk;
    IRrecv            mIrda;
    decode_results    mIrdaRes;
    ESP8266WebServer  mSrv;     // http server
    uByteLong         mIP;
        
    DNSServer         mDns;
        
    CGlobalData() : mSrv(80), mBtn( PIN_BUTTON ), mBlnk( PIN_LED ), mIrda(PIN_IRDA), 
                    mfCall(&CGlobalData::fInit), mfCallPrev(0)
    { Defaults();
    }

    void    Start(void);
    void    Run(void);
    void    RstOutlet(void) { mfCall = &CGlobalData::fPowerRst; };
    void    CheckAP( void );
    
    void    WebInit( void );
    void    WebLogin( void );
    void    WebTxPage( PGM_P content, const char* Title );
    void    Pgm2Str( String &s, PGM_P content );
    void    PingAsw( short Id, short SeqNo, short Ttl, const byte *Pkt );
    int     vWebVarFunc( char *Val, const VARMAP_ENTRY *e, bool bPrint );
    
    void  Defaults( void )
          { memset( &_St, 0, &_En-&_St );
            strcpy( mWfAP_Id,  "WebOutlet" ); // default WiFi SSID
            strcpy( mWfAP_Pwd, "weboutlet" ); // default WiFi password
	          strcpy( mWf_Name,  "WebOutlet" );	// Device hostname
            strcpy( mWf_Login, "Admin" );     // Login
            strcpy( mWf_Pwd,   "admin" );     // Password
            
            mRstIdle      = 10;   // Pause after Power Off, min
            mRstLength    = 15;   // Power Off sec
            mPingTTL      = 30;   // MAX Ping answer time sec
            mPingRetry    = 4;    // Retries before 'NoANSWER'
            mPingInterval = 60;   // Pause after Ping, sec
            mbPingerOn    = 1;    // Pinger ON
            mBlnkMode     = 0xFF; // All Ok
          }
};

extern CGlobalData gD;

int   PingSend( byte *IpAddrB4, short Id, short Seq );

