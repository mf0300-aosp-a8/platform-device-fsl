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
#	define DEBUG_MSG(e,...) LOGD("**[%s:%d]** " e, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#	define DEBUG_MSG(e,...) //
#endif

int main () {
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
	char szBuf[20]={0};

	int nSocket=socket_local_client("serialnumber", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
	send(nSocket, "cmd::get_phy_mem::", 18, 0);
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