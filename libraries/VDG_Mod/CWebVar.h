/* Copyright (C) 2017. All rights reserved.

 */

#ifndef _CWebVar_h_
#define _CWebVar_h_

// Defines for VAR_MAP( , , , Type ) 
#define VME_DEC		0x00	// Var - decimal (%d)
#define VME_HEX		0x10	// Var - HEX (%X)
#define VME_ASCII	0x20	// Var - ASCII (%s)
#define VME_FUNC	0x30	// Var - ptr to function to call (pFUNC_VM)
#define VME_IP		0x40	// Var - decimal IP (%d.%d.%d.%d)
#define VME_JUST0	0x80	// Var2Str() will justify str with '0'
#define VME_SIZEMASK    0x0F	// Output size to justify


struct VARMAP_ENTRY
{ const char *sName;	// Var name
        void *sVar;	// Var pointer		
	byte sSize;	// Var size
	byte sType;	// Var type VME_xxxx
};

class CWebVar
{ const VARMAP_ENTRY *GetThisMessageMap();

  virtual int	vWebVarFunc( char *Val, const VARMAP_ENTRY *e, bool bPrint ) { return 0; }

 public:
  static int Ip2Str( char *Str, byte *Ip ) { return sprintf( Str, "%d.%d.%d.%d", Ip[0], Ip[1], Ip[2], Ip[3]); }

  // Add variable VarName to String sTo. Convert it according to sType
  // 1: All ok, 0:Name not found or invalid data format	
  int	Var2Str( const char *VarName, String &sTo )
	{ int		i, n;
	  uint32_t	v;
	  char		Tb[256];
 	  for( const VARMAP_ENTRY *me = GetThisMessageMap(); me->sName; me++ )
	  { 	if( strcmp( VarName, me->sName ) ) continue;
                char *fmt = "%d";
		switch( me->sType & 0x70 )
		{ case VME_HEX:  fmt = "%X"; 
                  case VME_DEC:
		  	switch( me->sSize & 0x07 )	
		  	{ default: v = *((byte*)me->sVar); break;
                    	  case 2:  v = *((uint16_t*)me->sVar); break;
		    	  case 4:  v = *((uint32_t*)me->sVar); break;
		  	}
		  	sprintf( Tb, fmt, v );
		  	break;

		  case VME_ASCII:sprintf( Tb, "%s", me->sVar ); break;
		  case VME_IP:   Ip2Str( Tb,  (byte*)me->sVar ); break;
		  case VME_FUNC: if( !vWebVarFunc( Tb, me, 1 ) ) return 0;
				 break;
		}

		if( me->sType & VME_JUST0 )
		{ i = me->sType & 0x0F;
		  if( i ) 
		  { n = strlen( Tb );
		    if( n < i ) 
		    {   i -= n;
			memmove( Tb+i, Tb, n+1 );
			memset( Tb, '0', i );	  
	   	    }
		  }
		}
		sTo += Tb;
		return 1;		
	  }	  	
	  return 0;
	}

	// Put value Val into variable VarName. Convert it according to sType.
	// 1: All ok, 0:Name not found or invalid data format	
	int	Str2Var( const char *VarName, const char *Val )
	{ int		i, n = 0;
	  byte		b[8];
	  while( *Val && strchr( " \t", *Val ) ) Val++; // Skip spaces, tabs
 	  for( const VARMAP_ENTRY *me = GetThisMessageMap(); me->sName; me++ )
	  { 	if( strcmp( VarName, me->sName ) ) continue;
		i = 10;
		switch( me->sType & 0x70 )
		{ case VME_HEX:  i = 16;
		  case VME_DEC:  
			n = strtol( Val, 0, i );
			switch( me->sSize )	
		  	{ default: *((byte*)me->sVar) = n; break;
                    	  case 2:  *((uint16_t*)me->sVar) = n; break;
		    	  case 4:  *((uint32_t*)me->sVar) = n; break;
		  	}
			return 1;

		  case VME_ASCII: 
                        if( !me->sSize ) break;
			for( i=0,n = me->sSize-1; i<n && Val[i]; i++ ) ((byte*)me->sVar)[i] = Val[i];
			((byte*)me->sVar)[i] = 0;
			return 1;

		  case VME_IP:
			for( i=0; i<4; i++ )
			{ b[i] = strtol( Val, (char**)&Val, 10 );
			  if( !strchr( " .", *Val ) ) break;
			}
			if( i < 3 ) break;
			memcpy( me->sVar, b, 4 );
			return 1;

		 case VME_FUNC: if( !vWebVarFunc( (char*)Val, me, 0 ) ) return 0;
		}
		break;
	  }	  	
	  return 0;
	}

	// Call Srv.arg( VarName ) for all known vars and call 	Str2Var() for them.
	// 1: All ok, 0:Name not found or invalid data format	
	void	Form2Var( ESP8266WebServer &Srv )
	{ for( const VARMAP_ENTRY *me = GetThisMessageMap(); me->sName; me++ )
	  {  if( !Srv.hasArg( me->sName ) ) continue;       // check if argument exists
	     String s = Srv.arg( me->sName );
	     Str2Var( me->sName, s.c_str() );
	  }
	}

        
};

/* Defines for mapping  variables to names
 * BEGIN_VAR_MAP( CWebVar )				// Start map
   VAR_MAP( "PT", &PingTTL, sizeof(mPingTTL), VME_DEC )	// Name PT maps to PingTTL in decimal form
   VAR_MAP( "IP", &Addr, 4, VME_IP ) 			// Name IP maps to Addr in decimal IP address form. Example: 127.0.0.1
   VAR_MAP( "I+", &IRDA, 2, VME_HEX | VME_JUST0 | 4 )   // Name I+ maps to IRDA command in HEX form. Example: 02EF
   END_VAR_MAP()					// End of map declaration
*/


#define BEGIN_VAR_MAP(theClass) \
	const VARMAP_ENTRY *theClass::GetThisMessageMap() \
	{ \
		static const VARMAP_ENTRY _messageEntries[] =  \
		{

#define VAR_MAP( Name, VarPtr, Size, Type ) { (Name), (VarPtr), (Size), Type },

#define END_VAR_MAP() \
		{0, 0, 0, 0} \
		};\
		return _messageEntries;\
	};
	
#endif
