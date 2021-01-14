#!/bin/sh

# We do not use patch any more.
# See com/samsung/slsi/Patch.java

CURRENT_PATH=$1

cp $CURRENT_PATH/Looper.patch $ANDROID_BUILD_TOP
cd $ANDROID_BUILD_TOP
patch -p1 -N --dry-run --silent < ./Looper.patch 2>/dev/null
if [ $? -eq 0 ]; then
	echo "Apply patch to Looper.java"
	patch -p1 < ./Looper.patch
else
	rm -f $ANDROID_BUILD_TOP/frameworks/base/api/*.rej
	rm -f $ANDROID_BUILD_TOP/frameworks/base/core/java/android/os/*.rej
fi
rm -f $ANDROID_BUILD_TOP/Looper.patch
