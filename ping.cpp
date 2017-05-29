/*  WebOutlet v1.01 by VDG
 *  Remote PowerOutlet WEB control on base ESP8266 chip.
 *  Auto-reset power if there are no answer on pings. 
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  PING related stuff
 *
 *  24.05.2017 v1.01 created by VDG
 */
#include "main.h"

#include "IPAddress.h"
#include <functional>

extern "C" 
{
  #include <lwip/ip.h>
  #include <lwip/sys.h>
  #include <lwip/raw.h>
  #include <lwip/icmp.h>
  #include <lwip/inet_chksum.h>
}

#define PING_DATA_SIZE  64

static struct raw_pcb *pcbraw = 0;

u8_t ping_recv( void *arg, raw_pcb *pcb, pbuf *pb, ip_addr *addr )
{ struct ip_hdr *ip = (struct ip_hdr*)pb->payload;
  if( pbuf_header( pb, -PBUF_IP_HLEN ) == 0 )
  { struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr*)pb->payload;
    if( iecho->type == ICMP_ER ) 
    { gD.PingAsw( iecho->id, htons(iecho->seqno), ip->_ttl, ((byte*)iecho) + sizeof(struct icmp_echo_hdr) );
      pbuf_free( pb );
      return 1; /* eat the packet */
    }
  }
  pbuf_header( pb, PBUF_IP_HLEN );
  return 0; /* don't eat the packet */
}

//-------------------------------------------------------------------------------

struct raw_pcb *pcbGet( void )
{ if( !pcbraw ) 
  { pcbraw = raw_new(IP_PROTO_ICMP );
    raw_recv( pcbraw, ping_recv, 0 );
    raw_bind( pcbraw, IP_ADDR_ANY );
  }
  return pcbraw;
}

//-------------------------------------------------------------------------------

void    CGlobalData::PingAsw( short Id, short SeqNo, short Ttl, const byte *Pkt  )
{
  if( mPingPhase != 1 ) return; // Not expecting answer
  if( mPingSeqNo != SeqNo ) return; // Not expected pkt
  mPingMs = millis() - mPingMs;
  mPingPhase = 2;
}


//-------------------------------------------------------------------------------

int   PingSend( byte *IpAddrB4, short Id, short Seq ) 
{ uint32_t  long  tm;
  int       i, Rc = 0,
            Sz = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

  struct pbuf *pb = pbuf_alloc( PBUF_IP, Sz, PBUF_RAM );
  if( !pb ) 
    return 0;

  if( (pb->len == pb->tot_len) && (pb->next == NULL) ) 
  { struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)pb->payload;

    ICMPH_TYPE_SET( iecho, ICMP_ECHO );
    ICMPH_CODE_SET( iecho, 0 );
    iecho->chksum = 0;
    iecho->id = Id; // 0000-ffff
    iecho->seqno = htons( Seq ); // 0000-7fff
    
    /* fill the additional data buffer with some data */
    for(i = 0; i < PING_DATA_SIZE; i++)
      ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)(Seq+i);

    tm = millis();

//Serial.print( "#" ); Serial.print( iecho->id ); Serial.print( "-" ); Serial.print( Seq ); Serial.print( IpAddrB4[0] ); Serial.print( IpAddrB4[1] ); Serial.print( IpAddrB4[2] ); Serial.println( IpAddrB4[3] );

    iecho->chksum = inet_chksum(iecho, Sz );

    ip_addr_t addr;
    memcpy( &addr, IpAddrB4, 4 );    
    raw_sendto( pcbGet(), pb, &addr );
    Rc = 1;
  }
  pbuf_free(pb);
  return Rc;
}

//-------------------------------------------------------------------------------
/*
void    CGlobalData::PingCheck( void )
{ 
  int       i, n, bFin = 1;
  byte      *pb, *pt;
  uint16_t  us;
  uint32_t  ms = millis();

  switch( cp->mPhase )
  { case 0: // New cycle
      cp->mPhase++;   // Sending pings
      cp->mTime = ms;
      
    case 1: // Wait for idle
      if( ms - cp->mTime < cp->mInterval ) return; // Wait for interval before ping
      
      cp->mPhase++;
      cp->mTime = ms;      
      for( tsa=cp->mAddr; tsa; tsa=tsa->mNext ) // Clear data for new ping
      { if( tsa->mType != 8 ) continue; // Not a TSecAddr*
        tsa->mVal = 0;   // Reset fail counter
        tsa->mPhase = 0;
      }

    case 2: // Sending pings    
      bFin = 1;
      for( tsa=cp->mAddr; tsa; tsa=tsa->mNext )
      { if( tsa->mType != 8 ) continue; // Not a TSecAddr*
        switch( tsa->mPhase )
        { case 0: // Send new ping            
            bFin = 0;
            tsa->mPhase = 1; // Wait for data
            tsa->mTime = millis();
            if( ++mPingSeqNo & 0x8000 ) mPingSeqNo = 1;
            tsa->mSeqNo = mPingSeqNo;
            i = PingSend( tsa->mAddr, (long)tsa, tsa->mSeqNo, tsa );
            if( !i ) 
            {   tsa->mPhase = 4; // Failed to send
                continue;
            }
            
          case 1: // Waiting for PingAsw
            bFin = 0;
            if( ms - tsa->mTime < cp->mTTL ) continue; // Wait for PingAsw
            if( ++tsa->mVal >= cp->mTTLRetry )   // No answer
              tsa->mPhase = 3; // No answer received and all retries are spent
            else 
              tsa->mPhase = 0; // Retry to send ping
            continue;
        }     
      }      
  }

  if( !bFin ) return; // Some address is not answered yet
}
*/
//-------------------------------------------------------------------------------

