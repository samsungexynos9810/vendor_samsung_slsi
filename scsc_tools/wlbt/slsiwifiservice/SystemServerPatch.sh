#!/bin/sh

# We do not use patch any more.
# See com/samsung/slsi/Patch.java

CURRENT_PATH=$1

if ! [ -f $ANDROID_BUILD_TOP/frameworks/opt/net/wifi/service/java/com/android/server/wifi/SlsiWifiService.java ]; then
	ln -s $PWD/$CURRENT_PATH/SlsiWifiService.java $ANDROID_BUILD_TOP/frameworks/opt/net/wifi/service/java/com/android/server/wifi/SlsiWifiService.java
fi
if ! [ -f $ANDROID_BUILD_TOP/frameworks/opt/net/wifi/service/java/com/android/server/wifi/SlsiWifiSsidDecoder.java ]; then
	ln -s $PWD/$CURRENT_PATH/SlsiWifiSsidDecoder.java $ANDROID_BUILD_TOP/frameworks/opt/net/wifi/service/java/com/android/server/wifi/SlsiWifiSsidDecoder.java
fi

cp $CURRENT_PATH/SystemServer.patch $ANDROID_BUILD_TOP
cd $ANDROID_BUILD_TOP
patch -p1 -N --dry-run --silent < ./SystemServer.patch 2>/dev/null
if [ $? -eq 0 ]; then
	echo "Apply patch to SystemServer.java"
	patch -p1 < ./SystemServer.patch
else
	rm -f $ANDROID_BUILD_TOP/frameworks/base/services/java/com/android/server/*.rej
fi
rm -f $ANDROID_BUILD_TOP/SystemServer.patch
