#ifndef __SERIAL_NUMBER_DAEMON__
#define __SERIAL_NUMBER_DAEMON__

#undef LOG_TAG
#define LOG_TAG "FicSernd"

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <pthread.h>
#include <cutils/properties.h>

int cmd_parser(char *szCmd, int nSocket);
int set_serial(int nSocket, char *szSerial, int nLeng);
int get_serial(int nSocket, char *szSerial, int nLeng);
int change_mode(int nSocket, char *szSerial, int nLeng);
int reboot_device(int nSocket, char *szSerial, int nLeng);
int get_picversion(int nSocket, char *szSerial, int nLeng);
int get_phy_mem(int nSocket, char *szSerial, int nLeng);
int set_macaddr(int nSocket, char *szBuf, int nLeng);
int get_macaddr(int nSocket, char *szBuf, int nLeng);
int set_macaddr2(int nSocket, char *szBuf, int nLeng);
int get_macaddr2(int nSocket, char *szBuf, int nLeng);
int mac_to_eeprom(int nSocket, char *szBuf, int nLeng, int offset);
int mac_from_eeprom(int nSocket, char *szBuf, int nLeng, int offset);
int netcmd(int nSocket, char *szSerial, int nLeng);


#define _SERIAL_NUM_DAEMON_PORT		7652
#define _SERIAL_NUM_MAX_LEN		30
#define _I2C1_DEVICE_BASE		"/sys/devices/soc0/soc/2100000.aips-bus/21a0000.i2c"
#define _I2C2_DEVICE_BASE		"/sys/devices/soc0/soc/2100000.aips-bus/21a4000.i2c"
#define _I2C3_DEVICE_BASE		"/sys/devices/soc0/soc/2100000.aips-bus/21a8000.i2c"
#define _SERIAL_CMD			"cmd"
#define _SERIAL_SET			"set"
#define _SERIAL_GET			"get"
#define _SERIAL_RES			"res"
#define _CHANGE_MODE			"fic_chmod"
#define _SERIAL_SEPRATOR		"::"
#define _PIC_LABEL			"pic"

#define _FACTORY_REBOOT_DEVICE		"reboot"
#define	_PICVERSION_GET			"picversion"
#define _PHYSICAL_MEMORY_GET		"get_phy_mem"

#define _EEPROM_SIZE  			256
#define _MAC_OFFSET   			0x80
#define _MAC_OFFSET_2 			0x90
#define _MAC_SIZE       		6

#define _MAC_ADDR_SET  			"set_mac"
#define _MAC_ADDR_GET  			"get_mac"
#define _MAC_ADDR_SET_2  		"set_mac_2"
#define _MAC_ADDR_GET_2  		"get_mac_2"

#define _NET_CMD			"netcmd"
#define IFACE 				"eth0"

#define LOGD ALOGD

#endif
