#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <android/log.h>


#define DEBUG
#define LOG_TAG "test_sern"
#ifdef DEBUG
#	define DEBUG_MSG(e,...) ALOGD("**[%s:%d]** " e, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#	define DEBUG_MSG(e,...) //
#endif

int exists(const char *fname)
{
    int fd;
    if (fd = open(fname, O_RDONLY))
    {
        close(fd);
        return 1;
    }
    return 0;
}

void loop_exist(const char *fname){
	while(1){
		if (exists(fname))
			break;
		usleep(200*1000);		
	}
	return;	
}

int main (int argc, char* argv[]) {

	int fd;

	loop_exist("/dev/ttyUSB0");
	sleep(1);		
	loop_exist("/dev/ttyUSB1");
	sleep(1);		
	loop_exist("/dev/ttyUSB2");
	sleep(1);		
	loop_exist("/dev/ttyUSB3");
	sleep(5);		

	if (fd = open("/sys/class/leds/SP213_PWR_EN/brightness", O_RDWR))
	{
		write(fd, "1", 1);
		close(fd);
	}
	return 0;
}
