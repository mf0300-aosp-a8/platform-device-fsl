#include <sys/stat.h> 
#include <fcntl.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <android/log.h>
#include <sys/klog.h>
#include <time.h>
#include <hardware_legacy/uevent.h>
#include <netutils/ifc.h>

#define DEBUG

#ifdef DEBUG
#	define DEBUG_MSG(e,...) ALOGD("**[%s:%d]** " e, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#	define DEBUG_MSG(e,...)
#endif


#define ENET_ONLINE "online@/devices/platform/ethernet_led.0"
#define ENET_OFFLINE "offline@/devices/platform/ethernet_led.0"
#define IFACE "eth0"

static void handle_uevent(const char* udata)
{
    const char *str = udata;
	unsigned flags = 0;
	static int check = 0;

	if (!strncmp(str, ENET_ONLINE, strlen(ENET_ONLINE))) {

		ifc_get_info(IFACE, NULL, NULL, &flags);
		if (!check && flags) {
			ifc_disable(IFACE);
            check++;
        }
		
		ifc_enable(IFACE);
	}
	else if(!strncmp(str, ENET_OFFLINE, strlen(ENET_OFFLINE))) {

		ifc_disable(IFACE);
	}
	else
    	ALOGE("incorrect uevent : %s", str);

}

static void event_loop(void)
{
    int len = 0;
    static char udata[4096];
    memset(udata, 0, sizeof(udata));

    uevent_init();

    while (1) {
        len = uevent_next_event(udata, sizeof(udata) - 2);
        handle_uevent(udata);
    }
}

int main () {

	event_loop();
	return 0;
}
