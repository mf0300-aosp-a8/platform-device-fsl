#include "sernd.h"
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/reboot.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <android/log.h>
#include <sys/klog.h>
#include <dirent.h>
#include <time.h>
#include <net/if_arp.h>
#include <net/if.h>
//#include <vector>

#define MAX_CONNECTION 50

#define DEBUG

#ifdef DEBUG
#	define DEBUG_MSG(e,...) ALOGD("**[%s:%d]** " e, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#	define DEBUG_MSG(e,...) //
#endif

#define BUFFER_SIZE 2048
#define SEND_FAIL(nSocket) {send(nSocket, "FAIL\r\n", 6, 0);}
#define SEND_OK(nSocket) {send(nSocket, "OK\r\n", 4, 0);}
//  
//std::vector<int> g_vtConnection;
static char g_szSerial[40];
static char g_szMAC[6];
static unsigned long g_nPhyMem=0;

struct SCmdMap {
	char *szCmd;
	int (*fnCmd) (int, char *, int);
};

char eeprom_path[1024];

static bool list_dir (const char * dir_name)
{
    DIR * d;
    bool find_eeprom = 0;

    /* Open the directory specified by "dir_name". */
    d = opendir(dir_name);

    /* Check it was opened. */
    if (d == NULL) {
        DEBUG_MSG("Cannot open directory '%s': %s\n", dir_name, strerror (errno));
		return false;
    }
    while (1) {
        struct dirent * entry;
        const char * d_name;

        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir(d);
        if (entry == NULL) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = entry->d_name;
        /* Print the name of the file and directory. */
		// DEBUG_MSG("%s/%s %x\n", dir_name, d_name, entry->d_type);
        /* See if "entry" is a subdirectory of "d". */
        if (entry->d_type & DT_DIR) {
            /* Check that the directory is not "d" or d's parent. */
            if (strcmp (d_name, "..") != 0 &&
                strcmp (d_name, ".") != 0) {
                int path_length;
                char path[PATH_MAX];
 
                path_length = snprintf (path, PATH_MAX, "%s/%s", dir_name, d_name);
                if (path_length >= PATH_MAX) {
                    DEBUG_MSG("Path length has got too long.\n");
                    return false;
                }
                /* Recursively call "list_dir" with the new path. */
                if (list_dir (path)) {
                    find_eeprom = true;
                    break;
	        	}
            }
        } else {
	    	if (strcmp("eeprom", entry->d_name)==0) {
	            find_eeprom = true;
        		sprintf(eeprom_path, "%s/%s", dir_name, d_name);
            	DEBUG_MSG("%s\n", eeprom_path);
            	break;
	    	}
		}
    }
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
        DEBUG_MSG ("Could not close '%s': %s\n", dir_name, strerror (errno));
    }
    return find_eeprom;
} 

struct SCmdMap g_cmdMap[] = {
	{ const_cast<char*>(_PHYSICAL_MEMORY_GET), get_phy_mem },
	{ const_cast<char*>(_SERIAL_SET), set_serial },
	{ const_cast<char*>(_SERIAL_GET), get_serial },
	{ const_cast<char*>(_CHANGE_MODE), change_mode },
	{ const_cast<char*>(_FACTORY_REBOOT_DEVICE), reboot_device },
	{ const_cast<char*>(_MAC_ADDR_SET), set_macaddr },
	{ const_cast<char*>(_MAC_ADDR_GET), get_macaddr },
	{ const_cast<char*>(_MAC_ADDR_SET_2), set_macaddr2 },
	{ const_cast<char*>(_MAC_ADDR_GET_2), get_macaddr2 },
	{ const_cast<char*>(_NET_CMD), netcmd },
	{ reinterpret_cast<char*>(NULL), NULL },
};

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

int hwaddr_aton(const char *buf, unsigned char *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num(*buf++);
		if (a < 0)
			return -1;
		b = hex2num(*buf++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
		if (i < 5 && *buf++ != ':')
			return -1;
	}
	return 0;
}

int is_valid_mac(const unsigned char *macaddr) {
	return !(macaddr[0] & 0x01) && (macaddr[0] | macaddr[1] | macaddr[2] | macaddr[3] | macaddr[4] | macaddr[5]);
}

int set_macaddr(int nSocket, char *szBuf, int nLeng) {
	int ret = 0;
	int offset = _MAC_OFFSET;

	ret = mac_to_eeprom(nSocket, szBuf, nLeng, offset);
	return ret;
}

int set_macaddr2(int nSocket, char *szBuf, int nLeng) {
	int ret = 0;
	int offset = _MAC_OFFSET_2;

	ret = mac_to_eeprom(nSocket, szBuf, nLeng, offset);
	return ret;
}

int get_macaddr(int nSocket, char *szBuf, int nLeng) {
	int ret = 0;
	int offset = _MAC_OFFSET;

	ret = mac_from_eeprom(nSocket, szBuf, nLeng, offset);
	return ret;
}

int get_macaddr2(int nSocket, char *szBuf, int nLeng) {
	int ret = 0;
	int offset = _MAC_OFFSET_2;

	ret = mac_from_eeprom(nSocket, szBuf, nLeng, offset);
	return ret;
}

int mac_to_eeprom(int nSocket, char *szBuf, int nLeng, int offset) {

	unsigned char szMAC[_MAC_SIZE] = {0};
	unsigned char szRom[_EEPROM_SIZE] = {0};
	int i, nWrite = 0;

	if (0 == nLeng || NULL == szBuf) {
		DEBUG_MSG("null pointer or null size\n");
		return 0;
	}
	
	int fd = open(eeprom_path, O_RDWR);
	if (fd == 0) {
		DEBUG_MSG("open device: %s failed\n", eeprom_path);
		SEND_FAIL(nSocket);
		return 0;
	}
	int nRead = static_cast<int>(read(fd, szRom, _EEPROM_SIZE));
	close(fd);
	
	hwaddr_aton(szBuf, szMAC);

	DEBUG_MSG("set macaddr : %x%x%x%x%x%x\n", szMAC[0], szMAC[1], szMAC[2], szMAC[3] ,szMAC[4] ,szMAC[5]);

	if (!is_valid_mac(szMAC)) {
		DEBUG_MSG("invalid mac address.\n");
		SEND_FAIL(nSocket);
		return 0;
	}

	memcpy(szRom+offset, szMAC, _MAC_SIZE);

	fd = open(eeprom_path, O_RDWR);
	if (fd != 0) {
		nWrite = static_cast<int>(write(fd, szRom, _EEPROM_SIZE));
		close(fd);
		SEND_OK(nSocket);
		DEBUG_MSG("write mac : %d\n", nWrite);
	} else {
		SEND_FAIL(nSocket);
		DEBUG_MSG("write mac : %d ERROR\n", nWrite);
	}
	return nWrite;
}

int mac_from_eeprom(int nSocket, char *szBuf, int nLeng, int offset) {
	unsigned char szRom[_EEPROM_SIZE] = {0};
	char szRes[20] = {0};
	memset(g_szMAC, 0, sizeof(g_szMAC));
	
	int fd = open(eeprom_path, O_RDWR);
	if (fd == 0) {
		DEBUG_MSG("open device: %s failed\n", eeprom_path);
		SEND_FAIL(nSocket);
		return 0;
	}
	int nRead = static_cast<int>(read(fd, szRom, _EEPROM_SIZE)); close(fd);
	memcpy(g_szMAC, szRom+offset, _MAC_SIZE);
	sprintf(szRes, "%02x:%02x:%02x:%02x:%02x:%02x\r\n", g_szMAC[0], g_szMAC[1], g_szMAC[2], g_szMAC[3], g_szMAC[4], g_szMAC[5]);
	DEBUG_MSG("res: %s\n", szRes);
	if (nSocket) {
		send(nSocket, szRes, strlen(szRes), 0);
	}
	DEBUG_MSG("------\n");
	return nRead;
}

/*
* command format :
* [prefix]::[command]::[data]::
*
* ex. cmd::set::1234::
*/
int cmd_parser(char *szCmd, int nSocket) {

	uint ncmd = sizeof(g_cmdMap) / sizeof(SCmdMap);
	size_t i, j = 0;
	int datalen, cmdlen;
	char *tok;

	DEBUG_MSG("ncmd : %d\n", ncmd);

	/* get prefix token */
	tok = strtok(szCmd, _SERIAL_SEPRATOR);
	if (tok != NULL) {
		/* get command string token */
		tok = strtok(NULL, _SERIAL_SEPRATOR);
		for (i = 0; i < ncmd; i++) {
			cmdlen = strlen(g_cmdMap[i].szCmd) > strlen(tok) ? strlen(g_cmdMap[i].szCmd) : strlen(tok);
			/* parser command */
			if (!memcmp(g_cmdMap[i].szCmd, tok, cmdlen)) {
				/* command to set mac address */
				if (!strcmp(tok, _MAC_ADDR_SET) || !strcmp(tok, _MAC_ADDR_SET_2)) {
					char *macaddr;
					macaddr = (char*)malloc(20);
					memset(macaddr, 0, 20);

					while (tok != NULL && j<_MAC_SIZE) {
						if (j > 0) strncat(macaddr, ":", 1);
						tok = strtok(NULL, _SERIAL_SEPRATOR);
						strncat(macaddr, tok, 2);
						j++;
					}
					DEBUG_MSG("cmd: %s, data: %s\n", g_cmdMap[i].szCmd, macaddr);
					g_cmdMap[i].fnCmd(nSocket, macaddr, strlen(macaddr));
				} else {
					tok = strtok(NULL, _SERIAL_SEPRATOR);
					datalen = (tok != NULL) ? strlen(tok) : 0;
					DEBUG_MSG("cmd: %s, data: %s\n", g_cmdMap[i].szCmd, tok);
					g_cmdMap[i].fnCmd(nSocket, tok, datalen);
				}
				break;
			}
			if (i == ncmd-1) {
				DEBUG_MSG("command not found : %s\n", tok);
			}
		}
	}
	return 0;
}

int set_serial(int nSocket, char *szSerial, int nLeng) {
	if (0 == nLeng || NULL == szSerial) {
		DEBUG_MSG("null pointer or null size\n");
		return 0;
	}
	
	int fd = open(eeprom_path, O_RDWR);
	if (fd == 0) {
		DEBUG_MSG("open device: %s failed\n", eeprom_path);
		SEND_FAIL(nSocket);
		return 0;
	}
	szSerial[nLeng]=0;
	int nWrite = static_cast<int>(write(fd, szSerial, nLeng + 1 < _SERIAL_NUM_MAX_LEN ? nLeng+1 : _SERIAL_NUM_MAX_LEN));
	close(fd);
	SEND_OK(nSocket);
	return nWrite;
}

int get_serial(int nSocket, char *szSerial, int nLeng) {
	memset(g_szSerial, 0, sizeof(g_szSerial));
	int fd = open(eeprom_path, O_RDWR);
	if (fd == 0) {
		DEBUG_MSG("open device: %s failed\n", eeprom_path);
		SEND_FAIL(nSocket);
		return 0;
	}
	int nRead=static_cast<int>(read(fd, g_szSerial, _SERIAL_NUM_MAX_LEN));
	close(fd);
	
	char szRes[50];
	sprintf(szRes, "%s\r\n", g_szSerial);
	DEBUG_MSG("res: %s\n", szRes);
	if (nSocket > 0) {
		send(nSocket, szRes, strlen(szRes), 0);
		DEBUG_MSG("------\n");
	} else {
		DEBUG_MSG("------ nSocket ERROR\n");
	}
	return nRead;
}

int change_mode(int nSocket, char *szSerial, int nLeng) {
	int ret = 0, leng = 0;
	char * szTemp;
	char szRes[50] = {0};

	if (nLeng > 0 && szSerial != NULL) {
		leng = strlen(szSerial);
		szTemp = (char*)calloc(leng + 8, 1);
		if (szTemp) {
			sprintf(szTemp, "chmod %s\n", szSerial);
			DEBUG_MSG("cmd: %s\n", szTemp);
			ret = system(szTemp);
			DEBUG_MSG("system return: %d\n", ret);

			sprintf(szRes, "%d\r\n", ret);
			if (nSocket > 0) {
				send(nSocket, szRes, strlen(szRes), 0);
				DEBUG_MSG("------\n");
			} else {
				DEBUG_MSG("------ nSocket ERROR\n");
			}
			free(szTemp);
		} else {
			DEBUG_MSG("CALLOC ERROR\n");
		}
	}
	//memset(szTemp, 0, sizeof(szTemp));
	/* OLD METHOD
	sprintf(szTemp, "chmod %s\n", szSerial);
	DEBUG_MSG("cmd: %s\n", szTemp);
	ret = system(szTemp);
	DEBUG_MSG("system return: %d\n", ret);

	sprintf(szRes, "%d\r\n", ret);
	if (nSocket > 0) {
		send(nSocket, szRes, strlen(szRes), 0);
		DEBUG_MSG("------\n");
	} else {
		DEBUG_MSG("------ nSocket ERROR\n");
	}
	*/
	return ret;
}

int reboot_device(int nSocket, char *szSerial, int nLeng) {
	DEBUG_MSG("------ Rebooting~~~~~\n");	
	reboot(RB_AUTOBOOT);
	return 0;
}

int get_phy_mem(int nSocket, char *szSerial, int nLeng) {
	if (nSocket) {
		char szSendData[20]={0};
		sprintf(szSendData, "%lu", g_nPhyMem);
		DEBUG_MSG("total memory: %s\n", szSendData);
		send(nSocket, szSendData, strlen(szSendData), 0);
	}
	DEBUG_MSG("------\n");
	return 0;
}

char * findMemoryString(char* szMem) {
	char *szRes = NULL;
	if (NULL == szMem) {
		return szRes;
	}
	char *szEnd = strstr(szMem+1, "\n");
	if (NULL == szEnd) {
		DEBUG_MSG("not find end\n");
		return szRes;
	}
	
	const char *szFindStr[]= {
		"available",
		"reserved",
		"highmem",
		NULL
	};
	int i = 0;
	while (szFindStr[i]) {
		char *szFind = strstr(szMem+1, szFindStr[i]);
		if (szEnd < szFind) {
			DEBUG_MSG("not find %s\n", szFindStr[i]);
			return szRes;
		}	
		i++;
	}
	DEBUG_MSG("find the memory size string, then parser it\n");
	*szEnd = 0;
	DEBUG_MSG("memory string: %s\n", szMem);	
	// process the string
	char *szFind;
	do {
		szFind = strstr(szMem, ",");
		if (szFind) {
			*szFind = ' ';
		}
	} while (szFind);
	char szUsed[20] = {0}, szTotal[20] = {0}, szReserved[20] = {0}, szHighmem[20] = {0};
	// parser the number
	sscanf(szMem, "Memory: %[0-9]k/%[0-9]k available %[0-9]k reserved  %[0-9]K highmem", szUsed, szTotal, szReserved, szHighmem);
	DEBUG_MSG("%s %s %s %s\n", szUsed, szTotal, szReserved, szHighmem);
	g_nPhyMem = static_cast<unsigned long>(atoi(szTotal) + atoi(szReserved) + atoi(szHighmem));
	DEBUG_MSG("t: %d, r: %d, h: %d, p: %lu\n", atoi(szTotal), atoi(szReserved), atoi(szHighmem), g_nPhyMem);
	szRes = szMem;
	return szRes;
}

void * getPhyMemThread(void *pParam) {
	char szKernelMsg[128];
	DEBUG_MSG("++++++\n");
	int fd;
	fd = open("/proc/totalmemory", O_RDONLY|O_NONBLOCK);
	if (fd == 0) {
		DEBUG_MSG("open /proc/totalmemory failed\n");
		return 0;
	}

	read(fd, szKernelMsg, sizeof(szKernelMsg)-1);
	{
		char szUsed[20];
		sscanf(szKernelMsg, "%[0-9]k", szUsed);
		g_nPhyMem = static_cast<unsigned long>(atoi(szUsed));
	}
	if (fd) {
		close(fd);
	}
	DEBUG_MSG("------\n");
	return 0;
}


int netcmd(int nSocket, char *szSerial, int nLeng) 
{
	char szRes[50] = {0};
	int ret;
	szSerial[nLeng] = 0;
	if ((strncmp(szSerial, "ifconfig eth0", strlen("ifconfig eth0")) != 0)&&
		(strncmp(szSerial, "route", strlen("route")) != 0)){
		DEBUG_MSG("illegal command: %s\n", szSerial);
		return -1;
	}
	DEBUG_MSG("cmd: %s\n", szSerial);
	ret = system(szSerial);
	DEBUG_MSG("system return: %d\n", ret);
	sprintf(szRes, "%d\r\n", ret);
	if (nSocket) {
		send(nSocket, szRes, strlen(szRes), 0);
	} else {
		DEBUG_MSG("------ nSocket ERROR\n");
	}
	return ret;
}	

void * actionThread(void *pParam) {
	int cbrecv = 0;
	int nSocket = reinterpret_cast<int>(pParam);

	// Variable stack allocation can cause stack overflow, use heap now
	char * sz = (char*)calloc(BUFFER_SIZE, 1);
	if (sz != NULL) {
		cbrecv = recv(nSocket, sz, (BUFFER_SIZE - 1), 0); 
		if (cbrecv > 0) {
			sz[cbrecv] = 0;
			DEBUG_MSG("recv %i: %s\n", cbrecv, sz);
			cmd_parser(sz, nSocket);
		} else {
			DEBUG_MSG("recv %i DROPPED\n", cbrecv);
		}
		//g_vtConnection.erase(nSocket);
		free(sz);
	} else {
		DEBUG_MSG("CALLOC ERROR\n");
	}
	close(nSocket);	
	DEBUG_MSG("------ terminated\n");
	return 0;
}

int mac_from_eeprom_for_ethernet_device(void)
{
	unsigned char szRom[_EEPROM_SIZE] = {0};
	unsigned char * szMac;
	int fd;
	struct ifreq ifr;
	int s;
	int i;

	fd = open(eeprom_path, O_RDWR);
	if (fd == 0) {
		DEBUG_MSG("open device: %s failed\n", eeprom_path);
		return 0;
	}
	int nRead = static_cast<int>(read(fd, szRom, _EEPROM_SIZE)); close(fd);

	szMac = szRom + _MAC_OFFSET; 

	char szRes[20];
	sprintf(szRes, "%02x:%02x:%02x:%02x:%02x:%02x", szMac[0], szMac[1], szMac[2], szMac[3], szMac[4], szMac[5]);
	DEBUG_MSG("res: %s\n", szRes);
	if (is_valid_mac(szMac)) {
		if ((s = socket(PF_INET,SOCK_STREAM,0))!= -1)
		{
			strcpy(ifr.ifr_name, "eth0");
			for (i = 0; i < 6; i++)
				ifr.ifr_hwaddr.sa_data[i] = szMac[i];
			ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
			ioctl(s, SIOCSIFHWADDR, &ifr);
		}
	}
	return 0;
}

#ifdef MAC_AND_SERIAL_CHECK
struct Mac_range {
	unsigned long min;
	unsigned long max;
};

struct Mac_range mac_range[] = {{0x0b861d05, 0x0b8620ed},{0x0b8620ee, 0x0b863475},{0x0b865b86, 0x0b86b945}};

bool Check_Empty(unsigned char* buf, unsigned char pattern, int romsize)
{
	int i;
	for (i=0; i<romsize; i++){
		if (buf[i]!=pattern)
			return false;
	}
	return true;
}

//True : For FIC board or development
//False: For other vendor create
int CheckMotherBoard(void) 
{
	unsigned char szRom[_EEPROM_SIZE] = {0};
	unsigned long mac_number;
	int range_size, i;
	bool jump_flag = false;

	int fd = open(eeprom_path, O_RDWR);
	if (fd == 0) {
		DEBUG_MSG("open device: %s failed\n", eeprom_path);
		return 0;
	}
	int nRead = static_cast<int>(read(fd, szRom, _EEPROM_SIZE));
	close(fd);

	if(Check_Empty(szRom, 0x0, _EEPROM_SIZE))
		return 1;
	if(Check_Empty(szRom, 0xFF, _EEPROM_SIZE))
		return 1;

	DEBUG_MSG("Serial Number:%s\n", szRom);
	DEBUG_MSG("Mac Address:%02x.%02x.%02x.%02x.%02x.%02x\n", 
		szRom[_MAC_OFFSET], szRom[_MAC_OFFSET+1], szRom[_MAC_OFFSET+2], 
		szRom[_MAC_OFFSET+3], szRom[_MAC_OFFSET+4], szRom[_MAC_OFFSET+5]);
#if 0
	//Remove 0nly 20150820
	if (szRom[1]!='F')
		return 0;
	if (strncmp((char*)szRom+3, "410", 3)!=0)
		return 0;
#endif
	if ((szRom[_MAC_OFFSET]!=0) || (szRom[_MAC_OFFSET+1] != 0x14))
		return 0;

	mac_number = (((long)szRom[_MAC_OFFSET+2])<<24) + 
		(((long)szRom[_MAC_OFFSET+3])<<16) + 
		(((long)szRom[_MAC_OFFSET+4])<<8) + 
		((long)szRom[_MAC_OFFSET+5]);

	range_size = sizeof(mac_range) / sizeof(mac_range[0]);

	// DEBUG_MSG("%x %x\n", mac_number, range_size);
	for (i = 0; i < range_size; i++) {
		if ((mac_number >= mac_range[i].min)&&(mac_number <= mac_range[i].max)) {
			// DEBUG_MSG("%x %x\n", mac_number, i);
			return 1;
		}
	}	
	return 0;
}


void *CheckTimeThread(void *pParam) {
    struct timespec t;
	struct timespec t_start;
	long time_offset;

	time_offset = 2400 + rand() % 2400;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

	do {
    	clock_gettime(CLOCK_MONOTONIC, &t);
		// DEBUG_MSG("%d %d\n", (int)(t.tv_sec - t_start.tv_sec), time_offset);
		if ((long)(t.tv_sec - t_start.tv_sec) > time_offset) {
        	property_set("ctl.stop", "zygote");
        	property_set("ctl.stop", "surfaceflinger");
			sleep(3);
        	property_set("ctl.start", "surfaceflinger");
        	property_set("ctl.start", "zygote");
			time_offset = 2400 + rand() % 4800;
			clock_gettime(CLOCK_MONOTONIC, &t_start);
		}
		sleep(100);
	} while(1);
	return 0;
}

#endif
int main () {
	//get_serial(NULL, NULL, NULL);
	struct sockaddr_un re_addr;

	if ((list_dir(_I2C1_DEVICE_BASE) == 0) && (list_dir(_I2C2_DEVICE_BASE) == 0) && (list_dir(_I2C3_DEVICE_BASE) == 0)) {
	   	DEBUG_MSG("No Find\n");
        return 0;
    }

	mac_from_eeprom_for_ethernet_device();
	pthread_t mem_threadid=0;
	if (pthread_create(&mem_threadid, NULL, getPhyMemThread, NULL) == 0) {
		DEBUG_MSG("memory thread created\n");
	}	
	
	int nSocket = android_get_control_socket("serialnumber");
	if (listen(nSocket, MAX_CONNECTION) < 0) {
		DEBUG_MSG("listen failed\n");
		return 3;
	}

#ifdef MAC_AND_SERIAL_CHECK
	if (CheckMotherBoard() == 0) {
		pthread_t check_threadid = 0;
		if (pthread_create(&check_threadid, NULL, CheckTimeThread, NULL) == 0) {
			DEBUG_MSG("CheckTime thread created\n");
		}
	}	
#endif

	while (1) {		
		int nSinLeng = sizeof(struct sockaddr_un);
		int nClientSocket = accept(nSocket, (struct sockaddr *)&re_addr, &nSinLeng);
		if (nClientSocket < 0) {
			DEBUG_MSG("accept error\n");
			continue;
		}
		pthread_t threadid = 0;
		if (pthread_create(&threadid, NULL, actionThread, reinterpret_cast<void*>(nClientSocket)) == 0) {
			//g_vtConnection.push_back(nClientSocket);
			DEBUG_MSG("thread created:\n");
		}
	}
	return 0;
}
