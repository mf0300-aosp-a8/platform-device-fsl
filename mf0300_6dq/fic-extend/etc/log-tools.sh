PATH_BASE=/cache/log
CMD_ARGU=$1

case $CMD_ARGU in
        "get")
                rm /sdcard/log.tar.gz
                busybox tar czvf /sdcard/log.tar.gz $PATH_BASE

                ;;
        "clear")
                setprop logman stop
                rm $PATH_BASE/logcat/*
                rm $PATH_BASE/kernel/*
				
                ;;
esac

setprop logtools none
