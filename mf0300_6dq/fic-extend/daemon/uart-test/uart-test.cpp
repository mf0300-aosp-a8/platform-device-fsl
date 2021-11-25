#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/reboot.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <android/log.h>
#include <sys/klog.h>
#include <time.h>
#define LOG_TAG "uart-t"

#define MAX_CONNECTION 50

#define DEBUG

#ifdef DEBUG
#	define DEBUG_MSG(e,...) fprintf(stderr, "**[%s:%d]** " e, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#   define DEBUG_MSG //
#endif

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#define _POSIX_SOURCE 1         //POSIX compliant source
#define FALSE 0
#define TRUE 1

enum _PARAM {
	P_DEV	=	1,
	P_BAUD,
	P_DATABIT,
	P_PRIORITY,
	P_STOPBIT,
	P_FLOW,
};

enum _FLOW {
	F_NONE,
	F_HW,
	F_SW,
};

int main(int argc, char *argv[])
{
	char szDev[20]={0};
	int nBaud=0;
	int nDataBit=0;
	int nStopBit=0;
	int nPriority=0;
	int nPri=0;
	int nFlow=0;
	
	if (argc == 1) {
		strcpy(szDev, "/dev/ttymxc1");
		nBaud=115200;
		nDataBit=8;
		nStopBit=1;
		nPriority=0;
		nFlow=F_NONE;
	}
	else {
		strcpy(szDev, argv[P_DEV]);
		sscanf(argv[P_BAUD],"%li", &nBaud);
		sscanf(argv[P_DATABIT],"%i",&nDataBit);	
		sscanf(argv[P_STOPBIT],"%i",&nStopBit);	
		sscanf(argv[P_PRIORITY],"%i",&nPriority);
		sscanf(argv[P_FLOW],"%i",&nFlow);	
	}
	
	DEBUG_MSG("dev: %s\n", szDev);
	DEBUG_MSG("baud: %d\n", nBaud);
	DEBUG_MSG("data: %d\n", nDataBit);
	DEBUG_MSG("stop: %d\n", nStopBit);
	DEBUG_MSG("priority: %d\n", nPriority);
	DEBUG_MSG("flow: %d\n", nFlow);
	
	switch (nBaud)
	{
		case 115200:
		default:
			nBaud = B115200;
			break;
		case 38400:
			nBaud = B38400;
			break;
		case 19200:
			nBaud  = B19200;
			break;
	}  //end of switch baud_rate
	switch (nDataBit)
	{
		case 8:
			default:
			nDataBit = CS8;
		break;
			case 7:
			nDataBit = CS7;
		break;
	}  //end of switch data_bits
	switch (nStopBit)
	{
		case 1:
		default:
			nStopBit = CSTOPB;
			break;
		case 0:
			nStopBit = 0;
			break;
	}  //end of switch stop bits
	switch (nPriority)
	{
		case 0:
		default:                       //none
			nPriority = 0;
			nPri = 0;
			break;
		case 1:                        //odd
			nPriority = PARENB;
			nPri = PARODD;
			break;
		case 2:                        //even
			nPriority = PARENB;
			nPri = 0;
			break;
	}  //end of switch parity
	
	switch (nFlow) {
		default:
		case F_NONE:
			nFlow=0;
			break;
		case F_HW:
			nFlow=CRTSCTS;
			break;
		case F_SW:
			nFlow=CRTSCTS;
			break;
	}
       
	//open the device(com port) to be non-blocking (read will return immediately)
	int fd = open(szDev, O_RDWR | O_NOCTTY /*| O_NONBLOCK*/);
	if (fd < 0)
	{
		 perror(szDev);
		 return -1;
	}

	struct termios oldtio, newtio;
	bzero(&newtio, sizeof(newtio));

	tcgetattr(fd,&oldtio); // save current port settings 
	// set new port settings for canonical input processing 
	newtio.c_cflag = nBaud | nDataBit | nStopBit | nPriority | nFlow | nPri | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0; //ICANON;
	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=0;
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	
	unsigned char ar_dbBuf[] = {0x01, 0x03, 0x0c, 0x00};

#if 1	
	for (int i='a'; i<='z'; ++i) {
		DEBUG_MSG("write: %c\n", i);
		write(fd, &i, 1);
		usleep(70*1000);
		char ch[10]={0};
		read(fd, ch, sizeof(ch)-1);
		DEBUG_MSG("read: %s\n", ch);
	}
#else
	DEBUG_MSG("write data leng: %d\n", sizeof(ar_dbBuf));
	write(fd, ar_dbBuf, sizeof(ar_dbBuf));
	usleep(100*1000);
	
	unsigned char ar_dbRes[256]={0};
	
	int nLeng=0;
	do {
		memset (ar_dbRes, 0, sizeof(ar_dbRes));
		nLeng=read(fd, ar_dbRes, sizeof(ar_dbRes)-1);
	} while (nLeng != 0);
	
	DEBUG_MSG("read data -------------------------------\n");
	for (int i=0; i<nLeng;++i) {
		fprintf(stderr, "%02x ", ar_dbRes[i]);
	}
	fprintf(stderr, "\n");
	DEBUG_MSG("-----------------------------------------\n");
#endif
	close(fd);        //close the com port

	return 0;
}  //end of main

