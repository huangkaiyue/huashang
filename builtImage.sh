
TAR="localserver"

if [ "$1" = "" ];then
	echo input image name ...
	exit
fi


if [ "$1" = "tang" ];then
	make 
	SDK_PATH="/nfs/tang/"
	echo cp localserver to nfs
fi

if [ "$1" = "source" ];then
	make 
	SDK_PATH="/home/7620sdk/7688/sdk4300_20140916/RT288x_SDK/source/romfs/bin/"
	echo cp localserver to source
fi

cp $TAR $SDK_PATH
rm $TAR
echo .................. cp $TAR $SDK_PATH ok .................
