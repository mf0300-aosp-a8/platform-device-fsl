#include "sernd.h"
#include "test_inject.h"

#include <iostream> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <android/log.h>
#include <asm/ptrace.h>
#include <unistd.h>

#define MAX_CONNECTION 128

#define DEBUG

#ifdef DEBUG
#define DEBUG_MSGLC(e,...) {ALOGD("[D][%s:%d] " e, __FUNCTION__, __LINE__, ##__VA_ARGS__);printf("[D][%s:%d] " e, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define DEBUG_MSG(e,...)   {printf("[D][%s:%d] " e, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#else
#define DEBUG_MSGLC(e,...) //
#define DEBUG_MSG(e,...) //
#endif

pid_t _pid;

char TST1_S[] = "TEST INJECT SYSPROC: PASSED\n";
char TST1_F[] = "TEST INJECT SYSPROC: FAILED\n";
char TST2_S[] = "TEST INJECT SERND: PASSED\n";
char TST2_F[] = "TEST INJECT SERND: FAILED\n";

/*
 * This method will open /proc/<pid>/maps and search for the specified
 * library base address.
 */
uintptr_t findLibrary(const char *library, pid_t pid = -1) {
    /*
     * TEST
     */

    char filename[0xFF] = {0}, buffer[1024] = {0};
    FILE *fp = NULL;
    uintptr_t address = 0;
    sprintf(filename, "/proc/%d/maps", pid == -1 ? _pid : pid);
    fp = fopen(filename, "rt");
    if (fp == NULL) {
        perror("fopen");
        goto done;
    }
    while(fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, library)) {
            address = (uintptr_t)strtoul(buffer, NULL, 16);
            goto done;
        }
    }

done:
    if (fp){
        fclose(fp);
    }
    return address;
}

/*
 * Compute the delta of the local and the remote modules and apply it to
 * the local address of the symbol ... BOOM, remote symbol address!
 */
void *findFunction(const char* library, void* local_addr){
    uintptr_t local_handle, remote_handle;
    local_handle = findLibrary(library, getpid());
    remote_handle = findLibrary(library);
    return (void *)( (uintptr_t)local_addr + (uintptr_t)remote_handle - (uintptr_t)local_handle );
}

/*
 *
unsigned long call(void *function, int nargs, ...) {
    int i = 0;
    struct pt_regs regs, rbackup;
    
    memset(&regs, sizeof(regs), 0);
    memset(&rbackup, sizeof(rbackup), 0);
    
    // get registers and backup them
    trace(PTRACE_GETREGS, 0, &regs);
    memcpy(&rbackup, &regs, sizeof(struct pt_regs));

    va_list vl;
    va_start(vl,nargs);
    for (i = 0; i < nargs; ++i) {
        unsigned long arg = va_arg(vl, long);
        // fill R0-R3 with the first 4 arguments
        if (i < 4) {
            regs.uregs[i] = arg;
        } else {
            regs.ARM_sp -= sizeof(long) ;
            write((size_t)regs.ARM_sp, (uint8_t *)&arg, sizeof(long));
        }
    }
    va_end(vl);

    regs.ARM_lr = 0;
    regs.ARM_pc = (long int)function;
    // setup the current processor status register
    if ( regs.ARM_pc & 1 ) {
        // thumb
        regs.ARM_pc   &= (~1u);
        regs.ARM_cpsr |= CPSR_T_MASK;
    }
    else{
        // arm
        regs.ARM_cpsr &= ~CPSR_T_MASK;
    }
    // do the call
    trace(PTRACE_SETREGS, 0, &regs);
    trace(PTRACE_CONT);
    waitpid(_pid, NULL, WUNTRACED);
    // get registers again, R0 holds the return value
    trace(PTRACE_GETREGS, 0, &regs);
    // restore original registers state
    trace(PTRACE_SETREGS, 0, &rbackup);
    return regs.ARM_r0;
}
 */

int main_inject(int argc, char **argv )
{
    if (argc < 2) {
        printf("@ Usage %s PID LIBRARY\n", argv[0]);
    }

    if (geteuid() != 0) {
        fprintf(stderr, "E *** This script must be executed as root.\n");
        return 1;
    }

    pid_t pid = atoi(argv[1]);
    std::string library = argv[2];
    if (pid == 0) {
        fprintf( stderr, "E *** Invaid PID %s\n", argv[1] );
        return 1;
    }

    Traced proc(pid);
    printf("@ Injecting library %s into process %d.\n\n", library.c_str(), pid);
    printf("@ Calling dlopen in target process ...\n");
    
    unsigned long dlret = proc.dlopen(library.c_str());
    
    printf("@ dlopen returned 0x%lX\n", dlret);
    if (dlret != 0) return 0;
    return dlret;
}

int main_sernd(int argc, char **argv )
{
    char szBuf[50] = {0};
    char sztmp[50] = {0};
    unsigned int iteration = 0, r;
    
    int nSocket;
    for (iteration = 0; iteration < 256; iteration++) {
        sprintf(sztmp, "cmd::get_mac::");
        DEBUG_MSG("cmd string: %s\n", sztmp);

        // Prepare socket
        nSocket = socket_local_client("serialnumber", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
        send(nSocket, sztmp, 50, 0);
        r = recv(nSocket, szBuf, sizeof(szBuf), 0);
        close(nSocket);
        if (r > 0) szBuf[r] = 0;

        DEBUG_MSG("res string(%u): %s\n", r, szBuf);

        usleep(100);

        sprintf(sztmp, "cmd::set_mac::%02x:%02x:%02x:%02x:%02x:%02x", 0x11, 0x22, 0xee, 0x66, 0xaa, iteration);
        DEBUG_MSG("cmd string: %s\n", sztmp);

        // Prepare socket
        nSocket = socket_local_client("serialnumber", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
        send(nSocket, sztmp, 50, 0);
        r = recv(nSocket, szBuf, sizeof(szBuf), 0);
        close(nSocket);
        if (r > 0) szBuf[r] = 0;

        DEBUG_MSG("res string(%u): %s\n", r, szBuf);

        usleep(100);
    }
    return 0;
}
int main (int argc, char* argv[]) 
{
    unsigned int res = 0, r, total_ok = 0, total_fail = 0;

	DEBUG_MSG("TEST VERSION 1.0");
	
    r = main_inject(argc, argv);
    if (r == 0) {
        DEBUG_MSG("TEST INJECT SYSPROC: PASSED\n");
        ++total_ok;
    } else {
        DEBUG_MSG("TEST INJECT SYSPROC: FAILED\n");
        ++total_fail;
    }

    r = main_sernd(argc, argv);
    if (r == 0) {
        DEBUG_MSG("TEST INJECT SERND: PASSED\n");
        ++total_ok;
    } else {
        DEBUG_MSG("TEST INJECT SERND: FAILED\n");
        ++total_fail;
    }

    DEBUG_MSG("TOTAL TESTS PASSED:%u FAILED:%u\n", total_ok, total_fail);
	return 0;
}
