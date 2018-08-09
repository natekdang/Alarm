

#ifndef __H_SIO__
#define __H_SIO__

#ifndef APIENTRY
#define APIENTRY    	__stdcall 
#endif

#ifndef RCODE
#define RCODE	    	int 
#define R_OK			0
#define R_FAIL			-1
#endif



#pragma pack(1)

typedef enum {
	SIO_DATABITS_4 = 4,	
	SIO_DATABITS_5 = 5,	
	SIO_DATABITS_6 = 6,	
	SIO_DATABITS_7 = 7,	
	SIO_DATABITS_8 = 8

} SIO_DATABITS_ENUM;



typedef enum {
	SIO_STOPBITS_0 = 0,
	SIO_STOPBITS_1 = 1,
	SIO_STOPBITS_1_5 = 15,
	SIO_STOPBITS_2 = 2

} SIO_STOPBITS_ENUM;



typedef enum {
	SIO_PARITY_NONE,
	SIO_PARITY_EVEN,
	SIO_PARITY_ODD

} SIO_PARITY_ENUM;





typedef struct {
	int					SioPort;

	int					SioPortAddr;
	int					SioPortIrq;

	int					Baudrate;
	SIO_DATABITS_ENUM	Databits;
	SIO_STOPBITS_ENUM	Stopbits;
	SIO_PARITY_ENUM		Parity;

	int					TxSize;
	int					RxSize;

} SIO_PORT_TYPE;

	

#pragma pack()	






void APIENTRY COMVERSION(
	int *				MajorVer,
	int *				MinorVer,
	int *				Release,
	char *				szName,
	int					uLength
);

void APIENTRY SIO_InitAll(
	void
);

void APIENTRY SIO_Shutdown(
	void
);

RCODE APIENTRY SIO_PortOpen(
	int					SioPort,
	SIO_PORT_TYPE *		SioPortType
);
 
RCODE APIENTRY SIO_PortClose(
	int					SioPort
);

RCODE APIENTRY SIO_PortWrite(
	int					SioPort,
	int					data
);

RCODE APIENTRY SIO_PortWriteBuffer(
	int					SioPort,
	int					NoChar,
	char *				buffer,
	int *				NoWritten
);

RCODE APIENTRY SIO_PortRead(
	int					SioPort,
	int *				Data
);

RCODE APIENTRY SIO_PortReadBuffer(
	int					SioPort,
	int					NoChar,
	char *				Data
);

RCODE APIENTRY SIO_PortInQueueSize(
	int					SioPort
);

RCODE APIENTRY SIO_PortOutQueueSize(
	int					SioPort
);

RCODE APIENTRY SIO_PortInQueueFlush(
	int					SioPort
);

RCODE APIENTRY SIO_PortOutQueueFlush(
	int					SioPort
);

								  

#endif


