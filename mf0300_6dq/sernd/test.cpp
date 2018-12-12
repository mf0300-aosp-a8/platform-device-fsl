#include "sernd.h"
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <android/log.h>

#define MAX_CONNECTION 50

#define DEBUG
#define LOG_TAG "test_sern"
#ifdef DEBUG
#	define DEBUG_MSG(e,...) ALOGD("**[%s:%d]** " e, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#	define DEBUG_MSG(e,...) //
#endif

int main (int argc, char* argv[]) {
#if 0
	struct sockaddr_in addr, re_addr;
	int nSocket=socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == nSocket) {
		DEBUG_MSG("socket: %d error\n", nSocket);
		return 1;
	}
	addr.sin_family=AF_INET;
	addr.sin_port=htons(7652);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if (-1 == connect(nSocket, reinterpret_cast<struct sockaddr *>(&addr), sizeof(struct sockaddr))) {
		DEBUG_MSG("connect error\n");
		return 1;
	}
#endif
	char szBuf[50]={0};
	char sztmp[50]={0};
	int nSocket=socket_local_client("serialnumber", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);

	sprintf(sztmp, "cmd::set_mac::%02x:%02x:%02x:%02x:%02x:%02x", 0x11, 0x22, 0xee, 0x66, 0xaa, 0xdd);
	//sprintf(sztmp, "cmd::get_mac::");
	DEBUG_MSG("cmd string: %s\n", sztmp);
	send(nSocket, sztmp, 50, 0);
	recv(nSocket, szBuf, sizeof(szBuf), 0);
	DEBUG_MSG("res string: %s\n", szBuf);
	//recv(nSocket, szBuf, sizeof(szBuf), 0);
#if 0
	DEBUG_MSG("res string: %s\n", szBuf);	
	// get serial
	send(nSocket, "cmd::get::", 10, 0);
	recv(nSocket, szBuf, sizeof(szBuf), 0);
	DEBUG_MSG("res string: %s\n", szBuf);
	// set serial
	memset (szBuf, 0, sizeof(szBuf));
	send(nSocket, "cmd::set::0", 11, 0);
	recv(nSocket, szBuf, sizeof(szBuf), 0);
	DEBUG_MSG("res string: %s\n", szBuf);	
#endif
	return 0;
}