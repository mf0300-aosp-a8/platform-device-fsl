PATH_BASE=/cache/log/logcat

FILE_LIST=$(busybox ls -r $PATH_BASE/)

for i in $FILE_LIST
do
	echo [change the file $i to $((i+1))]
	busybox mv $PATH_BASE/$i $PATH_BASE/$((i+1))
done

if [ -f $PATH_BASE/61 ]; 
then
	echo [delete the $PATH_BASE/61]
	chmod 777 $PATH_BASE/61
	busybox rm $PATH_BASE/61 
fi

logcat -v time -b radio -b main -f $PATH_BASE/10

