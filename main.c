#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <Windows.h>
#include <tchar.h>

//ALARM TIMES-------------------------
#define START		"080000"
#define BREAK		"100000"
#define BREAKOVER	"101500"
#define LUNCH1		"120000"
#define LUNCH2		"123000"
#define LUNCH3		"130000"
#define LEAVE		"163000"

//GLOBALS-----------------------------
char array[6];
int count;

//STATES------------------------------
enum States {Monitor, SetOn, SetOff} State; 

//HELPER FUNCTIONS--------------------
void delay(int numSeconds)
{
	int i; 
	int j;

	for (i = 0; i < numSeconds; ++i)
	{
		for (j = 0; j < 300500000; ++j);
	}
}

int compareTimeArray(char array[])
{
	if (strcmp(array, START) == 0)  
	{
		return 1; 
	}
	else if (strcmp(array, BREAK) == 0) 
	{
		return 1; 
	}
	else if (strcmp(array, BREAKOVER) == 0)  
	{
		return 1; 
	}
	else if (strcmp(array, LUNCH1) == 0) 
	{
		return 1; 
	}
	else if (strcmp(array, LUNCH2) == 0) 
	{
		return 1; 
	}
	else if (strcmp(array, LUNCH3) == 0)
	{
		return 1;  
	}
	else if (strcmp(array, LEAVE) == 0)  
	{
		return 1;  
	}
	else if (strcmp(array, "123000") == 0) //TEST BRANCH
	{
		return 1; 
	}
	else 
	{
		return 0;
	}
}

//TICK FUNCTION----------------------
void TickFct()
{
	//time variables
	time_t curtime; 
	struct tm *loctime;

	//serial variables
	HANDLE hComm;
	char onBuffer[]				= "ON\r";
	char offBuffer[]			= "OFF\r";
	BOOL Status; 
	DWORD onBytesToWrite;
	DWORD onBytesWritten		= 0; 
	DWORD offBytesToWrite; 
	DWORD offBytesWritten		= 0;
	DCB dcbParameters			= {0}; 
	COMMTIMEOUTS timeouts		= {0};

	onBytesToWrite				= sizeof(onBuffer);
	offBytesToWrite				= sizeof(offBuffer); 

	switch(State)
	{
		case Monitor:
			if (compareTimeArray(array))
			{
				count = 0; 
				printf("ON TEST\n");

				//*********OPEN SERIAL PORT
				hComm = CreateFile(_T("COM3"),							//port name
									GENERIC_READ | GENERIC_WRITE,		//read/write
									0,									//no sharing
									NULL,								//no security
									OPEN_EXISTING,						//opening existing port only
									0, //FILE_FLAG_OVERLAPPED,			//overlapped I/O, change to 0 for non overlapped
									NULL);								//null for comm devices
				if (hComm == INVALID_HANDLE_VALUE)
				{
					printf("Error in opening serial port\n"); 
				}
				else 
				{
					printf("Serial port opened successfully\n");
				}

				//************SET PARAMETERS 
				dcbParameters.DCBlength = sizeof(dcbParameters);

				Status = GetCommState(hComm, &dcbParameters);

				if (Status == FALSE)
				{
					printf("Error in GetCommState()\n");
				}
				dcbParameters.BaudRate	= CBR_115200;
				dcbParameters.ByteSize	= 8;
				dcbParameters.StopBits	= ONESTOPBIT;
				dcbParameters.Parity	= NOPARITY;

				Status = SetCommState(hComm, &dcbParameters);  //Configuring the port according to settings in DCB 
				if (Status == FALSE)
				{
					printf("Error in Setting DCB Structure\n");
				}

				//************SET TIMEOUTS
				timeouts.ReadIntervalTimeout			= 50;
				timeouts.ReadTotalTimeoutConstant		= 50;
				timeouts.ReadTotalTimeoutMultiplier		= 10;
				timeouts.WriteTotalTimeoutConstant		= 50;
				timeouts.WriteTotalTimeoutMultiplier	= 10;

				if (SetCommTimeouts(hComm, &timeouts) == FALSE)
				{
					printf("Error in Setting Time Outs\n");
				}

				//************WRITE TO SERIAL PORT
				Status = WriteFile(hComm, 
								   onBuffer, 
								   onBytesToWrite,
								   &onBytesWritten, 
								   NULL); 

				if (Status == TRUE)
				{
					printf("Serial Write Success\n");
				}
				else 
				{
					printf("Serial Write Failure\n");
				}

				//**************CLOSE SERIAL PORT
				CloseHandle(hComm); 

				State = SetOn;

				//TESTING DELAY DELETE LATER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				delay(100);
			}
			else
			{
				State = Monitor;
			} //outer if else branch
			break;

		case SetOn:
			if (count < 15)
			{
				delay(1);
				State = SetOn;
			}
			else
			{
				State = SetOff;
				count = 0; 
			}
			break;

		case SetOff:
			State = Monitor; 
			break;
	} //end first switch

	switch(State)
	{
		case Monitor:
			curtime = time(NULL); 
			loctime = localtime(&curtime); 
			strftime(array, 7, "%H%M%S", loctime);
			//printf("The clock time is: %s\n", array);
			delay(1); 
			break;

		case SetOn:
			++count;
			//printf("%i", count);
			break;

		case SetOff:
			printf("OFF TEST\n");

			//*********OPEN SERIAL PORT
			hComm = CreateFile(_T("COM3"),							//port name
								GENERIC_READ | GENERIC_WRITE,		//read/write
								0,									//no sharing
								NULL,								//no security
								OPEN_EXISTING,						//opening existing port only
								0, //FILE_FLAG_OVERLAPPED,			//overlapped I/O, change to 0 for non overlapped
								NULL);								//null for comm devices
			if (hComm == INVALID_HANDLE_VALUE)
			{
				printf("Error in opening serial port\n"); 
			}
			else 
			{
				printf("Serial port opened successfully\n");
			}

			//************SET PARAMETERS 
			dcbParameters.DCBlength = sizeof(dcbParameters);

			Status = GetCommState(hComm, &dcbParameters);

			if (Status == FALSE)
			{
				printf("Error in GetCommState()\n");
			}
			dcbParameters.BaudRate	= CBR_115200;
			dcbParameters.ByteSize	= 8;
			dcbParameters.StopBits	= ONESTOPBIT;
			dcbParameters.Parity	= NOPARITY;

			Status = SetCommState(hComm, &dcbParameters);  //Configuring the port according to settings in DCB 
			if (Status == FALSE)
			{
				printf("Error in Setting DCB Structure\n");
			}

			//************SET TIMEOUTS
			timeouts.ReadIntervalTimeout			= 50;
			timeouts.ReadTotalTimeoutConstant		= 50;
			timeouts.ReadTotalTimeoutMultiplier		= 10;
			timeouts.WriteTotalTimeoutConstant		= 50;
			timeouts.WriteTotalTimeoutMultiplier	= 10;

			if (SetCommTimeouts(hComm, &timeouts) == FALSE)
			{
				printf("Error in Setting Time Outs\n");
			}

			//************WRITE TO SERIAL PORT
			Status = WriteFile(hComm, 
							   offBuffer, 
							   offBytesToWrite,
							   &offBytesWritten, 
							   NULL); 

			if (Status == TRUE)
			{
				printf("Serial Write Success\n");
			}
			else 
			{
				printf("Serial Write Failure\n");
			}

			//**************CLOSE SERIAL PORT
			CloseHandle(hComm); 
			
			break;
	} //end second switch
}

//MAIN----------------------------
void main( void )
{
	State = Monitor; 
	
	while(1)
	{
		TickFct(); 
	}
} 
