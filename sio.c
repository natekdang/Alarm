#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "sio.h"






#define MAX_SIO_PORT	8




static int				MajorVersion = 3;
static int				MinorVersion = 0;
static int				ReleaseVersion = 2;

static char				SIOCOMName[ 128 ] = "QuintetNT Serial Communication";

static HANDLE   		SioHandle[ MAX_SIO_PORT ];
static int				SioTxBuffer[ MAX_SIO_PORT ];



static WORD sio_cvt_PortNo(
	int					PortNo,
	SIO_PORT_TYPE *		PortType
);

static long sio_cvt_Baudrate(
	int					Baudrate
);

static short sio_cvt_Databits(
	int					Databits
);

static short sio_cvt_Stopbits(
	int					Stopbits
);

static short sio_cvt_Parity(
	int					Parity
);



void APIENTRY COMVERSION(
	int *				MajorVer,
	int *				MinorVer,
	int *				Release,
	char *				szName,
	int					uLength
)
{
	* MajorVer = MajorVersion;
	* MinorVer = MinorVersion;
	* Release  = ReleaseVersion;

	memset( szName, 0, uLength );
	strncpy( szName, SIOCOMName, uLength - 1 );
	szName[ uLength - 1 ] = '\0';
}



void APIENTRY SIO_InitAll(
	void
)
{
	int					i;


	// Reset all the handle
	for( i = 0; i < MAX_SIO_PORT; i++ )
	{
		SioHandle[ i ] = NULL;
		SioTxBuffer[ i ] = 0;
	}
}



void APIENTRY SIO_Shutdown(
	void
)
{
	int					i;


	// Close all serial port
	for( i = 0; i < MAX_SIO_PORT; i++ )
	{
		if( SioHandle[ i ] != NULL )
		{
			CloseHandle( SioHandle[ i ] );
			SioHandle[ i ] = NULL;
			SioTxBuffer[ i ] = 0;
		}
	}
}


RCODE APIENTRY SIO_PortOpen(
	int					SioPort,
	SIO_PORT_TYPE *		SioPortType
)
{
	WORD				portnum;
	long				lSpeed;
	short				DataBits;
	short				Parity;
	short				StopBits;

	char 				CommPort[ 64 ];
	DCB					dcb;
	COMMTIMEOUTS		TimeOut;

	TCHAR *				pcCommPort = TEXT( "COM4" );

	HANDLE hComm;

	// Check for valid port No
	if( ( SioPort <= 0 ) ||
		( SioPort > MAX_SIO_PORT ) )
	{
		return( R_FAIL );
	}


	// Port number
	portnum = sio_cvt_PortNo( SioPort, SioPortType );
	sprintf( CommPort, "COM4", portnum );

	// Speed
	lSpeed = sio_cvt_Baudrate( SioPortType-> Baudrate );

	// Databits
	DataBits = sio_cvt_Databits( SioPortType-> Databits );

	// Stopbits
	StopBits = sio_cvt_Stopbits( SioPortType-> Stopbits );

	// Parity
	Parity = sio_cvt_Parity( SioPortType-> Parity );

	SioHandle[ SioPort - 1 ] = CreateFile(
									pcCommPort,
									GENERIC_READ | GENERIC_WRITE,
									0,
									NULL,
									OPEN_EXISTING,
									0,
									NULL );

	if( SioHandle[ SioPort - 1 ] == INVALID_HANDLE_VALUE )
	{
		SioHandle[ SioPort - 1 ] = NULL;
		return( R_FAIL );
	}


	// Set the size of buffer
	if( !SetupComm( SioHandle[ SioPort - 1 ], 
		 		    ( SioPortType-> RxSize ? SioPortType-> RxSize : 2048 ), 
				    ( SioPortType-> TxSize ? SioPortType-> TxSize : 2048 ) ) )
	{
		CloseHandle( SioHandle[ SioPort - 1 ] );
		SioHandle[ SioPort - 1 ] = NULL;
		return( R_FAIL );
	}


	// Get comm value
	if( !GetCommState( SioHandle[ SioPort - 1 ], &dcb ) )
	{
		CloseHandle( SioHandle[ SioPort - 1 ] );
		SioHandle[ SioPort - 1 ] = NULL;
		return( R_FAIL );
	}

	dcb.BaudRate =lSpeed;
	dcb.ByteSize = ( unsigned char ) DataBits;
	dcb.Parity = ( unsigned char ) Parity;
	dcb.StopBits = ( unsigned char ) StopBits;

	if( !SetCommState( SioHandle[ SioPort - 1 ], &dcb ) )
	{
		CloseHandle( SioHandle[ SioPort - 1 ] );
		SioHandle[ SioPort - 1 ] = NULL;
		return( R_FAIL );
	}

	// Set time out
	if( !GetCommTimeouts( SioHandle[ SioPort - 1 ], &TimeOut ) )
	{
		CloseHandle( SioHandle[ SioPort - 1 ] );
		SioHandle[ SioPort - 1 ] = NULL;
		return( R_FAIL );
	}

	TimeOut.ReadIntervalTimeout =  MAXDWORD;
	TimeOut.ReadTotalTimeoutMultiplier = MAXDWORD;
	TimeOut.ReadTotalTimeoutConstant = 0;
	if( !SetCommTimeouts( SioHandle[ SioPort - 1 ], &TimeOut ) )
	{
		CloseHandle( SioHandle[ SioPort - 1 ] );
		SioHandle[ SioPort - 1 ] = NULL;
		return( R_FAIL );
	}

	// TxBuffer
	SioTxBuffer[ SioPort - 1 ] = SioPortType-> TxSize ? SioPortType-> TxSize : 2048;

	return( R_OK );
}


RCODE APIENTRY SIO_PortClose(
	int					SioPort
)
{
	// Check for valid port No
	if( ( SioPort <= 0 ) ||
		( SioPort > MAX_SIO_PORT ) )
	{
		return( R_FAIL );
	}

	// Already close, OK
	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	// Reset the handle
	else
	{
		CloseHandle( SioHandle[ SioPort ] );
	}

	return( R_OK );
}




RCODE APIENTRY SIO_PortWrite(
	int					SioPort,
	int					data
)
{
	DWORD				NoWritten;


	// Check for valid port No
	if( ( SioPort <= 0 ) ||
		( SioPort > MAX_SIO_PORT ) )
	{
		return( R_FAIL );
	}

	// Already close, OK
	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	// Reset the handle
	else if( WriteFile( SioHandle[ SioPort ], ( LPCVOID ) &data, 1, &NoWritten, NULL ) )
	{
		return( R_OK );
	}

	return( R_FAIL );
}


RCODE APIENTRY SIO_PortWriteBuffer(
	int					SioPort,
	int					NoChar,
	char *				buffer,
	int *				NoWritten
)
{
	// Check for valid port No
	if( ( SioPort <= 0 ) ||
		( SioPort > MAX_SIO_PORT ) )
	{
		return( R_FAIL );
	}

	// Already close, OK
	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	// Reset the handle
	else if( WriteFile( SioHandle[ SioPort ], buffer, NoChar, ( LPDWORD ) NoWritten, NULL ) )
	{
		return( R_OK );
	}

	return( R_FAIL );
}


RCODE APIENTRY SIO_PortRead(
	int					SioPort,
	unsigned int *		Data
)
{
	char				szBuffer[ 16 ];
	DWORD				dwLength;

	

	// Check for valid port No
	if( ( SioPort <= 0 ) ||
		( SioPort > MAX_SIO_PORT ) )
	{
		return( R_FAIL );
	}

	// Already close, OK
	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	// Read it
	else if( ReadFile( SioHandle[ SioPort ], szBuffer, 1, &dwLength, NULL ) )
	{
		if( dwLength == 1 )
		{
			* Data = ( unsigned int ) szBuffer[ 0 ];
			* Data = * Data & 0x0FF;
			return( R_OK );
		}

		else
		{
			return( R_FAIL );
		}
	}

	return( R_FAIL );
}




RCODE APIENTRY SIO_PortInQueueSize(
	int					SioPort
)
{
	COMSTAT				ComStat ;
	DWORD				dwErrorFlags;
	

	if( SioPort <= 0 || SioPort > MAX_SIO_PORT )
	{
		return( R_FAIL );
	}

	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	else if( ClearCommError( SioHandle[ SioPort ],  &dwErrorFlags, &ComStat ) )
	{
		return( ComStat.cbInQue );
	}

	return( R_FAIL );
}

RCODE APIENTRY SIO_PortOutQueueSize(
	int					SioPort
)
{
	COMSTAT				ComStat ;
	DWORD				dwErrorFlags;

	if( SioPort <= 0 || SioPort > MAX_SIO_PORT )
	{
		return( R_FAIL );
	}

	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	else if( ClearCommError( SioHandle[ SioPort ],  &dwErrorFlags, &ComStat ) )
	{
		return( SioTxBuffer[ SioPort ] - ComStat.cbOutQue );
	}

	return( R_FAIL );
}



RCODE APIENTRY SIO_PortInQueueFlush(
	int					SioPort
)
{
	// Not allow port number
	if( SioPort <= 0 || SioPort > MAX_SIO_PORT )
	{
		return( R_FAIL );
	}

	// Already close, OK
	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	// Reset the handle
	else if( !PurgeComm( SioHandle[ SioPort ], PURGE_RXCLEAR ) )
	{
		return( R_FAIL );
	}

	return( R_OK );
}

RCODE APIENTRY SIO_PortOutQueueFlush(
	int					SioPort
)
{
	// Not allow port number
	if( SioPort <= 0 || SioPort > MAX_SIO_PORT )
	{
		return( R_FAIL );
	}

	// Already close, OK
	else if( SioHandle[ --SioPort ] == NULL )
	{
		return( R_FAIL );
	}

	// Reset the handle
	else if( !PurgeComm( SioHandle[ SioPort ], PURGE_TXCLEAR ) )
	{
		return( R_FAIL );
	}


	return( R_OK );
}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


static WORD sio_cvt_PortNo(
	int					PortNo,
	SIO_PORT_TYPE *		PortType
)
{
	return( PortNo );
}


static long sio_cvt_Baudrate(
	int					Baudrate
)
{
	return( ( long ) Baudrate );
}



static short sio_cvt_Databits(
	int					Databits
)
{
	switch( Databits )
	{
		case SIO_DATABITS_5:
			return( 5 );

		case SIO_DATABITS_6:
			return( 6 );

		case SIO_DATABITS_7:
			return( 7 );

		case SIO_DATABITS_8:
			return( 8 );
	}

	return( 8 );
}



static short sio_cvt_Stopbits(
	int					Stopbits
)
{
	switch( Stopbits )
	{
		case SIO_STOPBITS_0:
			return( ONESTOPBIT );

		case SIO_STOPBITS_1:
			return( ONESTOPBIT );

		case SIO_STOPBITS_1_5:
			return( ONE5STOPBITS );

		case SIO_STOPBITS_2:
			return( TWOSTOPBITS );
	}

	return( ONESTOPBIT );
}


static short sio_cvt_Parity(
	int					Parity
)
{
	switch( Parity )
	{
		case SIO_PARITY_NONE:
			return( NOPARITY );

		case SIO_PARITY_EVEN:
			return( EVENPARITY );

		case SIO_PARITY_ODD:
			return( ODDPARITY );
	}
	
	return( NOPARITY );
}





