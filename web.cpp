/*  WebOutlet v1.01 by VDG
 *  Remote PowerOutlet WEB control on base ESP8266 chip.
 *  Auto-reset power if there are no answer on pings. 
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  WEB related stuff and pages data
 *
 *  24.05.2017 v1.01 created by VDG
 */

#include "main.h"

char gTitle[64];

// Map variables <-> webpage text placeholders
BEGIN_VAR_MAP( CWebVar )
VAR_MAP( "mT", gTitle,  0, VME_ASCII )
VAR_MAP( "wM", &gD.mWf_Mode,  sizeof(gD.mWf_Mode), VME_FUNC )
VAR_MAP( "wN", &gD.mWfAP_Id,  sizeof(gD.mWfAP_Id), VME_ASCII )
VAR_MAP( "wP", &gD.mWfAP_Pwd, sizeof(gD.mWfAP_Pwd), VME_ASCII )
VAR_MAP( "wC", &gD.mWfCL_Id,  sizeof(gD.mWfCL_Id), VME_ASCII )
VAR_MAP( "wA", &gD.mWfCL_Pwd, sizeof(gD.mWfCL_Pwd), VME_ASCII )
VAR_MAP( "wL", &gD.mWf_Login, sizeof(gD.mWf_Login), VME_ASCII )
VAR_MAP( "wS", &gD.mWf_Pwd, sizeof(gD.mWf_Pwd), VME_ASCII )
VAR_MAP( "wH", &gD.mWf_Name, sizeof(gD.mWf_Name), VME_ASCII )
VAR_MAP( "IP", gD.mIP.uB, 0, VME_FUNC ) // Current TCP address
VAR_MAP( "IR", &gD.mIrCommand, sizeof(gD.mIrCommand), VME_HEX | VME_JUST0 | 4 ) // Last IR command
VAR_MAP( "pT", &gD.mPingTTL, sizeof(gD.mPingTTL), VME_DEC )
VAR_MAP( "pR", &gD.mPingRetry, sizeof(gD.mPingRetry), VME_DEC )
VAR_MAP( "pI", &gD.mPingInterval, sizeof(gD.mPingInterval), VME_DEC )
VAR_MAP( "pM", &gD.mbPingerOn, sizeof(gD.mbPingerOn), VME_FUNC )
VAR_MAP( "rO", &gD.mbPingerOn, sizeof(gD.mbPingerOn), VME_FUNC )
VAR_MAP( "rI", &gD.mRstIdle, sizeof(gD.mRstIdle), VME_DEC )
VAR_MAP( "rL", &gD.mRstLength, sizeof(gD.mRstLength), VME_DEC )
VAR_MAP( "A1", &gD.mAddr[0], 4, VME_IP ) // IP address
VAR_MAP( "A2", &gD.mAddr[1], 4, VME_IP ) // IP address
VAR_MAP( "A3", &gD.mAddr[2], 4, VME_IP ) // IP address
VAR_MAP( "A4", &gD.mAddr[3], 4, VME_IP ) // IP address
VAR_MAP( "I1", &gD.mIR_Commands[0], 2, VME_HEX | VME_JUST0 | 4 ) // IRDA command
VAR_MAP( "I2", &gD.mIR_Commands[1], 2, VME_HEX | VME_JUST0 | 4 ) // IRDA command
VAR_MAP( "I3", &gD.mIR_Commands[2], 2, VME_HEX | VME_JUST0 | 4 ) // IRDA command
VAR_MAP( "I4", &gD.mIR_Commands[1], 2, VME_HEX | VME_JUST0 | 4 ) // IRDA command
VAR_MAP( "I5", &gD.mIR_Commands[2], 2, VME_HEX | VME_JUST0 | 4 ) // IRDA command
END_VAR_MAP()


const char P_Set[] PROGMEM = R"(
[rc:0k]
)";

//-------------------------------------------------------------------------------------------
// Common page header with styles, common script, etc. Title passed as @#T@ variable and is read only
const char P_Hdr[] PROGMEM = R"(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<head><meta http-equiv="Content-Type" content="text/html; charset=win1251">
<meta name=viewport content="width=device-width, initial-scale=1.1">
<style>

h4{text-align: center;background: orange;margin: -2px 0 1em 0;}
body {font-size:14px;}
label {float:left; padding-right:10px;}
.fld{clear:both; text-align:right; line-height:25px;}
.frm{float:left;border: 2px solid #634f36;background: #f3f0e9;padding: 5px;margin:3px;}
input[type=number]{width: 5em;} 
</style>
<script type="text/javascript">
 function TxRq(url,fName,id)
 { var xhr = new XMLHttpRequest();
   xhr.open('POST',url,false);
   var fData = new FormData(document.forms.namedItem(fName));
   xhr.send(fData);
   if (xhr.status != 200) alert( "TxRq error - " + xhr.status + ': ' + xhr.statusText );
   else { if (id) document.getElementById(id).innerHTML = xhr.responseText; }
 }
)";

//-------------------------------------------------------------------------------------------

const char P_Login[] PROGMEM = R"(
 window.addEventListener("load",function(e)
 { document.getElementById("btC").addEventListener("click",bhC);
 })
 function bhC(){ TxRq("l","fL" ); }
</script></head>
<body><div class="frm"><h4>@mT@</h4>
<form name="fL" method="POST"><br/>
<div class="fld"><label for="lL">Login:</label><input type="text" name="lL" maxlength="16"></div><br/>
<div class="fld"><label for="lP">Password:</label><input type="password" name="lP" maxlength="32"</div><br/>&emsp;
<hr/> 
<div style="float:right;"><button id="btC">Submit</button></div>
</form></div>
</div>
</body></html>
)"; 
 
//-------------------------------------------------------------------------------------------

const char P_Main[] PROGMEM = R"(
 window.addEventListener("load",function(e)
 { document.getElementById("btR").addEventListener("click",bhR);
   document.getElementById("btO").addEventListener("click",bhO);
   document.getElementById("btF").addEventListener("click",bhF);
   document.getElementById("btC").addEventListener("click",bhC);
 })
 function bhR(){ TxRq("i","fI" ); }
 function bhO(){ TxRq("i","fI" ); }
 function bhF(){ TxRq("i","fI" ); }
 function bhC(){ TxRq("i","fI" ); }
</script></head><body>
<div class="frm"><h4>@mT@</h4>
<table width="100%" cellspacing="0" cellpadding="4">
<tr><td align="left" width="60">Address:</td><td>@IP@</td></tr>
<tr><td align="left">IR Code:</td><td>@IR@</td></tr>
<tr><td align="left">Pinger:</td><td>@pM@</td></tr>
<tr><td colspan="2" align="left"><div><form name="fI" method="POST">
<hr/>
&emsp;<button id="btR" name="bt" value="R">Rst</button>
&emsp;<button id="btO" name="bt" value="O">On</button>
&emsp;<button id="btF" name="bt" value="F">Off</button>
&emsp;<button id="btC" name="bt" value="C">Pinger</button>&emsp;
</form></div></td></tr>
</table>
<hr/> 
<div style="float:left;"><a href="w">WebConfig</a>&emsp;<a href="p">PingConfig</a></div>
</div>
</body></html>
)";

//-------------------------------------------------------------------------------------------

const char P_CfgWeb[] PROGMEM = R"(
 window.addEventListener("load",function(e)
 { document.getElementById("btC").addEventListener("click",bhC);
 })
 function bhC(){ TxRq("w","fW" ); }
</script></head><body>
<div class="frm"><h4>@mT@</h4>
<form name="fW" method="POST">
 <div class="fld"><label for="wM">WiFi Mode</label><select name="wM">@wM@</select></div>
 <hr>
 <div class="fld"><label for="wN">AP Net ID:</label><input type="text" name="wN" maxlength="15" value=@wN@></div>
 <div class="fld"><label for="wP">AP Password:</label><input type="text" name="wP" maxlength="15" value=@wP@></div>
 <hr>
 <div class="fld"><label for="wC">Clnt Net ID:</label><input type="text" name="wC" maxlength="15" value=@wC@></div>
 <div class="fld"><label for="wA">Clnt Password:</label><input type="text" name="wA" maxlength="15" value=@wA@></div>
 <hr>
 <div class="fld"><label for="wL">Login:</label><input type="text" name="wL" maxlength="15" value=@wL@></div>
 <div class="fld"><label for="wS">Password:</label><input type="text" name="wS" maxlength="31" value=@wS@></div> 
 <div class="fld"><label for="wH">Host Name:</label><input type="text" name="wH" maxlength="15" value=@wH@></div> 
 <hr>
 <div style="float:left;"><a href="i">Main</a>&emsp;<a href="p">PingConfig</a></div>
 <div style="float:right;"><button id="btC">Set params</button></div>
</form>
</div>
</body></html>
)";

//-------------------------------------------------------------------------------------------

const char P_CfgPing[] PROGMEM = R"(
 window.addEventListener("load",function(e)
 { document.getElementById("btC").addEventListener("click",bhC);
 })
 function bhC(){ TxRq("p","fP" ); }
</script></head><body>
<div class="frm"><h4>@mT@</h4>
<form name="fP" method="POST">
 <div class="fld"><label for="pT">Ping TTL (sec):</label><input type="number" style="width: 5em" name="pT" min="1" max="120" value=@pT@></div>
 <div class="fld"><label for="pR">Ping Retries:</label><input type="number" name="pR" min="1" max="32" value=@pR@></div>
 <div class="fld"><label for="pI">Ping Interval (sec):</label><input type="number" name="pI" min="1" max="3600" value=@pI@></div>
 <hr/> 
 <div class="fld"><label for="A1">Address1:</label><input type="text" name="A1" maxlength="15" value=@A1@></div>
 <div class="fld"><label for="A2">Address2:</label><input type="text" name="A2" maxlength="15" value=@A2@></div>
 <div class="fld"><label for="A3">Address3:</label><input type="text" name="A3" maxlength="15" value=@A3@></div>
 <div class="fld"><label for="A4">Address4:</label><input type="text" name="A4" maxlength="15" value=@A4@></div>
 <hr/>
 <div class="fld"><label for="rO">Pinger ON:</label><input type="checkbox" name="rO" @rO@></div> 
 <div class="fld"><label for="rL">Restart PowerOff (sec):</label><input type="number" name="rL" min="1" max="600" value=@rL@></div> 
 <div class="fld"><label for="rI">Post Restart Delay (min):</label><input type="number" name="rI" min="1" max="120" value=@rI@></div> 
 <hr/>
 <div class="fld"><label for="I1">IRDA On:</label><input type="text" name="I1" maxlength="4" value=@I1@></div>
 <div class="fld"><label for="I2">IRDA Off:</label><input type="text" name="I2" maxlength="4" value=@I2@></div>
 <div class="fld"><label for="I3">IRDA Restart:</label><input type="text" name="I3" maxlength="4" value=@I3@></div>
 <div class="fld"><label for="I4">IRDA Ping ON:</label><input type="text" name="I4" maxlength="4" value=@I4@></div>
 <div class="fld"><label for="I5">IRDA Ping OFF:</label><input type="text" name="I5" maxlength="4" value=@I5@></div>
 <hr/> 
 <div style="float:left;"><a href="i">Main</a>&emsp;<a href="w">WebConfig</a></div>
 <div style="float:right;"><button id="btC">Set params</button></div>
</form>
</div>
</body></html>
)";

//-------------------------------------------------------------------------------------------

void  ShowArgs( const char *Msg )
{   Serial.print("\nUrl:" ); Serial.print( gD.mSrv.uri() ); Serial.print("<-" ); Serial.print( Msg );
    for ( int n = 0; n < gD.mSrv.args(); n++ ) 
     { Serial.print("\n"); Serial.print( n ); Serial.print(":"); 
       Serial.print(gD.mSrv.argName(n)); Serial.print("="); 
       Serial.print(gD.mSrv.arg(n)); 
     }
}

//-----------------------------------------------------------------------------

const int MAX_PHSIZE = 10; // Maximum placeholder name size including @@

void  CGlobalData::Pgm2Str( String &sPg, PGM_P content )
{   int     n, offset = 0;
    PGM_P   contentNext;
    char    chunk[400], Tb[64], *b, *p;
    
    while( true )
    {   // due to the memccpy signature, lots of casts are needed
        memset( chunk+offset, 0, sizeof(chunk)-offset );
        contentNext = (PGM_P)memccpy_P((void*)(chunk+offset), (PGM_VOID_P)content, 0, sizeof(chunk)-MAX_PHSIZE-1 );
        
        content += sizeof(chunk)-MAX_PHSIZE-1;

        p = chunk;
        offset = 0;
        while( 1 )
        { b = p;
          p = strchr( b, '@' );
          if( !p ) // No placeholders. Add all 
          { sPg += b;
            break;
          }
          *p = 0;
          sPg += b; // Add all before @
          b = p + 1;
          p = strchr( b, '@' );
          if( !p ) // No closing @ => not a placeholder
          { n = sizeof(chunk) - (b - chunk);
            if( n < MAX_PHSIZE ) // Probably torn placeholder
            { chunk[0] = '@';
              strcpy( chunk+1, b );
              offset = strlen( chunk );
              break;
            }
            sPg += "@";
            sPg += b;
            break;
          }
          n = p - b;
          if( n >= MAX_PHSIZE ) // Too far. 
          { sPg += "@";
            p = b; 
            continue;
          }
          
          memcpy( Tb, b, n ); 
          Tb[n] = 0;
          p++;

          Var2Str( Tb, sPg );
        }
        if( contentNext ) break;
    }
}

//-------------------------------------------------------------------------------

// Complex mapping processing for placeholders
int  CGlobalData::vWebVarFunc( char *Val, const VARMAP_ENTRY *e, bool bPrint )
{ 
  if( !strcmp( e->sName, "wM" ) )
  { if( bPrint )
    { const char *Nm[] = { "Access Point","Client",0 };
      for( int n=0; Nm[n]; n++ )
      { sprintf( Val, "<option value=\"%d\" %s>%s</option>", n, n==mWf_Mode ? "selected":"", Nm[n] );
      } 
    } else mWf_Mode = mWf_ModeCur = atoi(Val);
    return 1;
  }
          
  if( !strcmp( e->sName, "pM" ) )   // Running mode
  { if( bPrint )  
    { if( !mWf_ModeCur )
      { strcpy( Val, "<div style=\"background-color: #FC8436;\";>[Off] AP config mode</div>" );
        return 1;
      }
      
      if( !mbPingerOn )
      { strcpy( Val, "<div style=\"background-color: #FC8436;\";>[Off] Ping off</div>" );
        return 1;
      }
              
      if( mAddrCntr ) 
      { strcpy( Val, "<div style=\"background-color: #FFE646\";>[On] Some IP failed</div>" );
        return 1;
      }
              
      if( mPingCntr > 1 ) 
      { strcpy( Val, "<div style=\"background-color: #EFFF2F;\";>[On] Some ping lost</div>" );
        return 1;
      }
      
      strcpy( Val, "<div style=\"background-color: #87FF2F;\";>[On] All OK.</div>" );
      return 1;
    }  
  }
  
  if( !strcmp( e->sName, "rO" ) )   // Pinger ON
  { if( bPrint )  
    { if( mbPingerOn ) strcpy( Val, "checked" );
      return 1;
    } else
    { mbPingerOn = strcmp( Val, "on" ) == 0;      
    }
  }

  if( !strcmp( e->sName, "IP" ) )   // IP addr
  { if( bPrint )  
    { CWebVar::Ip2Str( Val,  mIP.uB );
      if( mWf_ModeCur ) strcat( Val, " / [STA]" ); else strcat( Val, " / [AP]" );
      return 1;
    } 
  }
}

//-------------------------------------------------------------------------------

void CGlobalData::WebTxPage( PGM_P content, const char *Title )
{   String  sPg;

    sprintf( gTitle, "%s:%s", mWf_Name, Title );   
    Pgm2Str( sPg, P_Hdr );
    Pgm2Str( sPg, content );

    if( mWebSessId[0] ) // Login assepted. Send session ID 
    { char  Tb[32];
      sprintf( Tb, "SID=%s", mWebSessId );
      mSrv.sendHeader("Cache-Control","no-cache");    
      mSrv.sendHeader("Set-Cookie",Tb);
    }
//Serial.println( sPg );    
    mSrv.send( 200, "text/html", sPg );
    mWebSessTm = millis();
}

//-------------------------------------------------------------------------------

void handle404() 
{ ShowArgs( "404" );
  String sPg = "File Not Found\n\nURI: ";
  sPg += gD.mSrv.uri();
  sPg += "\nMethod: ";
  sPg += ( gD.mSrv.method() == HTTP_GET ) ? "GET" : "POST";
  sPg += "\nArguments: ";
  sPg += gD.mSrv.args();
  sPg += "\n";

  for ( uint8_t i = 0; i < gD.mSrv.args(); i++ )
    sPg += "\n" + gD.mSrv.argName ( i ) + ":" + gD.mSrv.arg ( i );
//Serial.println( sPg );    
  gD.mSrv.send( 404, "text/plain", sPg );
}

//-------------------------------------------------------------------------------

int  CheckLogin( void )
{ while( gD.mWebSessId[0] ) // Is session ID
  { if( !gD.mSrv.hasHeader("Cookie") ) break;
    String cookie = gD.mSrv.header("Cookie");
    if (cookie.indexOf(gD.mWebSessId) < 0 ) break;
    gD.mWebSessTm = millis();
    return 1;
  }
  gD.WebLogin();
  return 0;
}

//-------------------------------------------------------------------------------

void handle_info() 
{ if( !CheckLogin() ) return;
 
  String s = gD.mSrv.arg( "bt" ) + '?';
  switch( s[0] )
  { case 'R': // Send RESET signal
      gD.RstOutlet();
      break;
    case 'O': // Relay ON
      gD.mbPingerOn = 0;
      digitalWrite( PIN_RELAY, 1 );
      break;
    case 'F': // Realy OFF
      gD.mbPingerOn = 0;
      digitalWrite( PIN_RELAY, 0 );
      break;
    case 'C': // Toggle ping check
      gD.mbPingerOn = !gD.mbPingerOn;
      break;
  }
  //ShowArgs( "Root" ); // Debug WEB output  
  gD.WebTxPage( P_Main, "WebOutlet info & control" );
}

void handle_cfgW() 
{ gD.Form2Var( gD.mSrv );
  VM_FlashWr( 0, &gD._St, &gD._Wr-&gD._St );
  //ShowArgs( "WebCfg" );  // Debug WEB output
  gD.WebTxPage( P_CfgWeb, "WebOutlet WiFi & Login config" );
}

void handle_cfgP() 
{ gD.Form2Var( gD.mSrv );
  VM_FlashWr( 0, &gD._St, &gD._Wr-&gD._St );
  //ShowArgs( "PingCfg" );  // Debug WEB output
  gD.WebTxPage( P_CfgPing, "WebOutlet Ping config" );
}

//-------------------------------------------------------------------------------

void  CGlobalData::WebInit( void )
{  gTitle[0] = 0;
   mSrv.onNotFound(handle404);
   mSrv.on("/", handle_info);
   mSrv.on("/i", handle_info);
   mSrv.on("/w", handle_cfgW);
   mSrv.on("/p", handle_cfgP);
   mSrv.on("/l", [](){ gD.WebLogin(); } );
   
   //ask server to track these headers
   const char * headerkeys[] = {"Cookie"};
   mSrv.collectHeaders(headerkeys, sizeof(headerkeys)/sizeof(char*) );
   mSrv.begin();
}

//-------------------------------------------------------------------------------

void  CGlobalData::WebLogin( void )
{ int   i;
  long  l;
  byte  mac[6], *pb;
  
  ShowArgs( "Login" ); // Debug WEB output
  while( 1 )
  { String s = mSrv.arg( "lL" );
    if( stricmp( s.c_str(), mWf_Login ) ) break;
    s = mSrv.arg( "lP" );
    if( stricmp( s.c_str(), mWf_Pwd ) ) break;

    // Create random session ID
    WiFi.macAddress( mac );
    l = millis();
    pb = (byte*)&l;
    for( i=0; i<4; i++ ) mac[i] ^= *pb++;
    l = micros();
    pb = (byte*)&l;
    for( i=2; i<6; i++ ) mac[i] ^= *pb++;
    for( i=0; i<6; i++ ) sprintf( mWebSessId+i*2, "%02X", mac[i] );
    mWebSessTm = millis();    
    WebTxPage( P_Main, "WebOutlet info & control" );
    return;
  }
  mWebSessId[0] = 0;
  WebTxPage( P_Login, "WebOutlet Login" );
  //delay(10000);
}

