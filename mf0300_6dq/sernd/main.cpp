#include "hook.h"
#include "hooks/io.h"

typedef struct
{
    const char *name;
    uintptr_t   original;
    uintptr_t   hook;
}
hook_t;

static hook_t __hooks[] = {

    ADDHOOK( open ),
    ADDHOOK( write ),
    ADDHOOK( read ),
    ADDHOOK( close ),
    ADDHOOK( connect ),
    ADDHOOK( send ),
    ADDHOOK( sendto ),
    ADDHOOK( sendmsg ),
    ADDHOOK( recv ),
    ADDHOOK( recvfrom ),
    ADDHOOK( recvmsg ),
    ADDHOOK( shutdown )
};

#define NHOOKS ( sizeof(__hooks) / sizeof(__hooks[0] ) )

uintptr_t find_original(const char *name) {
    for( size_t i = 0; i < NHOOKS; ++i ) {
        if( strcmp( __hooks[i].name, name ) == 0 ){
            return __hooks[i].original;
        }
    }

    HOOKLOG( "[%d] !!! COULD NOT FIND ORIGINAL POINTER OF FUNCTION '%s' !!!", getpid(), name );

    return 0;
}

void __attribute__ ((constructor)) libhook_main()
{
    HOOKLOG( "LIBRARY LOADED FROM PID %d.", getpid() );

    // get a list of all loaded modules inside this process.
    ld_modules_t modules = libhook_get_modules();

    HOOKLOG( "Found %u loaded modules.", modules.size() );
    HOOKLOG( "Installing %u hooks.", NHOOKS );

    for( ld_modules_t::const_iterator i = modules.begin(), e = modules.end(); i != e; ++i ){
        // don't hook ourself :P
        if( i->name.find( "libhook.so" ) == std::string::npos ) {
            HOOKLOG( "[0x%X] Hooking %s ...", i->address, i->name.c_str() );

            for( size_t j = 0; j < NHOOKS; ++j ) {
                unsigned tmp = libhook_addhook( i->name.c_str(), __hooks[j].name, __hooks[j].hook );

                // update the original pointer only if the reference we found is valid
                // and the pointer itself doesn't have a value yet.
                if( __hooks[j].original == 0 && tmp != 0 ){
                    __hooks[j].original = (uintptr_t)tmp;

                    HOOKLOG( "  %s - 0x%x -> 0x%x", __hooks[j].name, __hooks[j].original, __hooks[j].hook );
                }
            }
        }
    }
}
