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
	int i						= 0;
	int j						= 0;
	int timer					= 0; 
	HANDLE hComm;
	char tempChar				= ' '; 
	char onBuffer[]				= "ON\r";
	char offBuffer[]			= "OFF\r";
	char SerialBuffer[256]		= {0};  
	BOOL Status;
	DWORD onBytesToWrite;
	DWORD onBytesWritten		= 0; 
	DWORD offBytesToWrite; 
	DWORD offBytesWritten		= 0;
	DWORD numBytesRead			= 0; 
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
								   onBytesToWrite - 1, //don't write '/0'
								   &onBytesWritten, 
								   NULL); 

				if (Status == FALSE)
				{
					printf("Serial Write Failure\n");
				}
				
				//*************READ SERIAL PORT 
				tempChar = ' '; //reset tempChar
				timer = 0; 
				while (tempChar != '\n' && timer < 1000)
				{
					ReadFile( hComm, 
							  &tempChar, 
							  sizeof(tempChar), 
							  &numBytesRead, 
							  NULL); 
					if (numBytesRead == 1)
					{
						SerialBuffer[i] = tempChar;
						++i;
					}
					++timer; 
				}
                
                //**************PRINT TO CONSOLE 
                for (j = 0; j < i; ++j)				
                {									
                    printf("%c", SerialBuffer[j]);
                }

				//**************CLOSE SERIAL PORT
				CloseHandle(hComm);
				i = 0;
				j = 0;

				State = SetOn;
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

		default: 
			State = Monitor; 
			break; 
	} //end first switch

	switch(State)
	{
		case Monitor:
			curtime = time(NULL); 
			loctime = localtime(&curtime); 
			strftime(array, 7, "%H%M%S", loctime);
			delay(1); 
			break;

		case SetOn:
			++count;
			break;

		case SetOff:
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
							   offBytesToWrite - 1,
							   &offBytesWritten, 
							   NULL); 

			if (Status == FALSE)
			{
				printf("Serial Write Failure\n");
			}

			//**************READ SERIAL PORT
			tempChar = ' '; //reset tempChar
			timer = 0; 
			while (tempChar != '\n' && timer < 1000)
			{
				ReadFile( hComm, 
						  &tempChar, 
						  sizeof(tempChar), 
						  &numBytesRead, 
						  NULL); 
				if (numBytesRead == 1)
				{
					//printf("Read port success");  
					SerialBuffer[i] = tempChar;
					++i;
				}
				++timer; 
			}
                 
            //**************PRINT TO CONSOLE
            for (j = 0; j < i; ++j)
            {
                printf("%c", SerialBuffer[j]);
            } 

			//**************CLOSE SERIAL PORT
			CloseHandle(hComm);
			i = 0; 
			j = 0; 
			break;

		default: 
			State = Monitor;
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
