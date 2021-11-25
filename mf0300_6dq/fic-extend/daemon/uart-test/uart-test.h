#ifndef __SERIAL_NUMBER_DAEMON__
#define __SERIAL_NUMBER_DAEMON__

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <pthread.h>

int cmd_parser(char *szCmd, int nSocket);
int set_serial(int nSocket, char *szSerial, int nLeng);
int get_serial(int nSocket, char *szSerial, int nLeng);
int reboot_device(int nSocket, char *szSerial, int nLeng);
int get_picversion(int nSocket, char *szSerial, int nLeng);
int get_phy_mem(int nSocket, char *szSerial, int nLeng);

#define _SERIAL_NUM_DAEMON_PORT		7652
#define _SERIAL_NUM_MAX_LEN			30
#define _SERIAL_NODE_NAME			"/proc/factoryserial"
#define _PICVERSION_NODE_NAME			"/proc/picversion"
#define _SERIAL_CMD					"cmd"
#define _SERIAL_SET					"set"
#define _SERIAL_GET					"get"
#define _SERIAL_RES					"res"
#define _SERIAL_SEPRATOR			"::"
#define _PIC_LABEL				"pic"

#define _FACTORY_REBOOT_DEVICE		"reboot"
#define	_PICVERSION_GET			"picversion"
#define _PHYSICAL_MEMORY_GET		"get_phy_mem"

#endif
