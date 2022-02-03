#!/bin/bash

####### Functions for Print #######
color_title=$'\E'"[0;33m"
color_success=$'\E'"[0;32m"
color_error=$'\E'"[0;31m"
color_reset=$'\E'"[00m"

build_carrier=("chnopen" "cmcc" "europen" "vzw" "tmo" "att" "spr" "ctc" "cu" "kddi" "ntt" "global")

function elapsed_time() {
  let "ELAPSED_TIME=$END_TIME-$START_TIME"
  echo "${color_success}[BUILD TOOL]${color_reset} $FUNC: ${color_title}$ELAPSED_TIME seconds${color_reset}"
}

function print_title() {
  echo
  echo "[[[[[[[${color_title} $TITLE ${color_reset}]]]]]]]"
}

function print_error() {
  echo ${color_error}ERROR${color_reset}: $1
}

####### Arguments Check-up #######
function print_usage_exit () {
  print_error "$1"
  echo 'Usage: ./build.sh <PRODUCT> [ eng | userdebug | user ] [ "" | bootimage | host-tools | platform ] [ <TARGET_BUILD_CARRIER> ]'
  echo "                Available TARGET_BUILD_CARRIER : ${build_carrier[@]}"
  exit 2
}

function array_contains () {
  local seeking=$1; shift
  local in=1
  for element; do
    if [[ $element == $seeking ]]; then
      in=0
      break
    fi
  done
  echo $in
}

if [ $# -lt 1 ]; then
  print_usage_exit "need at least two arguments"
fi

####### Environment setup #######
if [ "z${CPU_JOB_NUM}" == "z" ] ; then
  CPU_JOB_NUM=$(grep processor /proc/cpuinfo | awk '{field=$NF};END{print (field+1)/2}')
fi
CLIENT=$(whoami)

ROOT_DIR=$(pwd)

BUILDSH=vendor/samsung_slsi/script/build.sh
BUILD_PRODUCT=$1
BUILD_VARIANT=$2
BUILD_OPTION=$3
BUILD_OPTION2=$4
OUT_DIR="out_${1}_${2}"
ESSI_SYSTEM_IMG=""
AVBTOOL=$ROOT_DIR/external/avb/avbtool

if [ -z $BUILD_VARIANT ]; then
  BUILD_VARIANT=eng
fi

MKDTIMG=$ROOT_DIR/host_tools/host/linux-x86/bin/mkdtimg
LPMAKE=$ROOT_DIR/host_tools/host/linux-x86/bin/lpmake
UFDT_APPLY_OVERLAY=$ROOT_DIR/host_tools/host/linux-x86/bin/ufdt_apply_overlay
BUILD_SUPER_IMAGE=$ROOT_DIR/build/make/tools/releasetools/build_super_image.py

function clean_android()
{
  TITLE="Clean android platform"
  print_title
  echo "rm -rf out$1/target/product/*/system"
  rm -rf out$1/target/product/*/system
  echo "rm -rf out$1/target/product/*/vendor"
  rm -rf out$1/target/product/*/vendor
  echo "rm -rf out$1/target/product/*/ramdisk"
  rm -rf out$1/target/product/*/ramdisk
  echo "rm -rf out$1/target/product/*/recovery"
  rm -rf out$1/target/product/*/recovery
  echo "rm -rf out$1/target/product/*/root"
  rm -rf out$1/target/product/*/root
  echo "rm -rf out$1/target/product/*/*.img"
  rm -rf out$1/target/product/*/*.img
  echo "rm -rf out$1/target/product/*/kernel"
  rm -rf out$1/target/product/*/kernel
  echo "rm -rf out$1/target/product/*/ff_*"
  rm -rf out$1/target/product/*/ff_*
}

if [ "clean" == "$1" ]; then
  clean_android "*"
  exit 0
fi

req_build_variant=(eng user userdebug)
if [ $(array_contains $2 "${req_build_variant[@]}") == 1 ]; then
  print_usage_exit "\"$2\" is not a build variant"
fi


####### Error Conditions #######
function check_exit()
{
  RET=$?
  if [ $RET != 0 ]
  then
    exit $RET
  fi
}

function check_environment()
{
  if [[ ! -z "${TARGET_PRODUCT}" ]]; then
    print_error "\"build/envsetup.sh\" and \"lunch\" should not be executed. Please re-create your shell"
    exit 2
  fi
}

check_environment

####### Collection Sequencess #######
function copy_binlist()
{
  IFS=' ' read -ra BINARR <<< $(echo "$BINLIST" | tr "\n" " ")
  for bin in "${BINARR[@]}"; do
    IFS=':' read -ra split <<< $bin
    if [ ! -f $BINBASE/${split[0]} ]; then
      print_error "$BINBASE/${split[0]}: No such file"
      exit 2
    fi
    cp -f $BINBASE/${split[0]} $DIST/${split[1]}
  done
}

function build_host_tools()
{

  if [ ! -z $(which $MKDTIMG) ]; then
    if [ ! -z $(which $LPMAKE) ]; then
      if [ ! -z $(which $UFDT_APPLY_OVERLAY) ]; then
        return 0
      fi
    fi
  fi
  TITLE="Build Host Tools"
  print_title
  START_TIME=`date +%s`
  echo "source build/envsetup.sh"
  source build/envsetup.sh
  echo
  echo "OUT_DIR=host_tools make mkdtimg lpmake ufdt_apply_overlay -j$CPU_JOB_NUM"
  OUT_DIR=host_tools make mkdtimg lpmake ufdt_apply_overlay -j$CPU_JOB_NUM
  echo
  check_exit
  END_TIME=`date +%s`
  FUNC=${FUNCNAME[0]}
  elapsed_time
}

function build_essi()
{
  TITLE="Build ESSI"
  print_title
  START_TIME=`date +%s`
  echo "source build/envsetup.sh"
  source build/envsetup.sh
  echo
  echo "lunch essi-$BUILD_VARIANT"
  OUT_DIR=out_essi_$BUILD_VARIANT lunch essi-$BUILD_VARIANT > /dev/null
  check_exit
  echo
  echo "make -j$CPU_JOB_NUM"
  echo
  OUT_DIR=out_essi_$BUILD_VARIANT make -j$CPU_JOB_NUM
  check_exit
  PRODUCT_OUT=$(OUT_DIR=out_essi_$BUILD_VARIANT get_build_var PRODUCT_OUT)
  ESSI_SYSTEM_IMG=$PRODUCT_OUT/system.img
  ls -l $ESSI_SYSTEM_IMG 2> /dev/null
  END_TIME=`date +%s`
  FUNC=${FUNCNAME[0]}
  elapsed_time
}

function create_vbmeta()
{
  echo "Build vbmeta"
  BOARD_AVB_ALGORITHM=$(OUT_DIR=$OUT_DIR get_build_var BOARD_AVB_ALGORITHM)
  BOARD_AVB_KEY_PATH=$(OUT_DIR=$OUT_DIR get_build_var BOARD_AVB_KEY_PATH)
  echo "ESSI: $ESSI_SYSTEM_IMG"
  echo "Partial Board Build: $PRODUCT_OUT"
  echo "Signing algorithm: $BOARD_AVB_ALGORITHM"
  echo "Key path: $BOARD_AVB_KEY_PATH"
  $AVBTOOL make_vbmeta_image \
	  --output $PRODUCT_OUT/vbmeta.img \
	  --key $BOARD_AVB_KEY_PATH \
	  --algorithm $BOARD_AVB_ALGORITHM \
	  --include_descriptors_from_image $ESSI_SYSTEM_IMG \
	  --include_descriptors_from_image $PRODUCT_OUT/vendor.img \
	  --include_descriptors_from_image $PRODUCT_OUT/boot.img \
	  --include_descriptors_from_image $PRODUCT_OUT/dtbo.img
}

function create_super()
{
  BOARD_SUPER_PARTITION_SIZE=$(OUT_DIR=$OUT_DIR get_build_var BOARD_SUPER_PARTITION_SIZE)
  BOARD_GROUP_BASIC_SIZE=$(OUT_DIR=$OUT_DIR get_build_var BOARD_GROUP_BASIC_SIZE)
  SUPER_ENV=$PRODUCT_OUT/super_env
  echo "use_dynamic_partitions=true"                          >  $SUPER_ENV
  echo "lpmake=$LPMAKE"                                       >> $SUPER_ENV
  echo "build_super_partition=true"                           >> $SUPER_ENV
  echo "super_metadata_device=super"                          >> $SUPER_ENV
  echo "super_block_devices=super"                            >> $SUPER_ENV
  echo "super_super_device_size=$BOARD_SUPER_PARTITION_SIZE"  >> $SUPER_ENV
  echo "dynamic_partition_list=system vendor"                 >> $SUPER_ENV
  echo "super_partition_groups=group_basic"                   >> $SUPER_ENV
  echo "super_group_basic_group_size=$BOARD_GROUP_BASIC_SIZE" >> $SUPER_ENV
  echo "super_group_basic_partition_list=system vendor"       >> $SUPER_ENV
  echo "system_image=$ESSI_SYSTEM_IMG"                        >> $SUPER_ENV
  echo "vendor_image=$PRODUCT_OUT/vendor.img"                 >> $SUPER_ENV

  $BUILD_SUPER_IMAGE -v \
        $PRODUCT_OUT/super_env \
        $PRODUCT_OUT/super.img
}

function build_chub()
{
  if [[ $1 == 1 ]]
  then
    echo "***********************************************************************"
    echo "source build/envsetup.sh"
    source build/envsetup.sh
    echo
    echo "lunch full_$BUILD_PRODUCT-$BUILD_VARIANT"
    OUT_DIR=$OUT_DIR lunch full_$BUILD_PRODUCT-$BUILD_VARIANT > /dev/null
    echo "***********************************************************************"
  fi

  NANOHUB_TARGET=$(OUT_DIR=$OUT_DIR get_build_var NANOHUB_TARGET)
  NANOHUB_PATH=$(OUT_DIR=$OUT_DIR get_build_var NANOHUB_PATH)/firmware
  TARGET_DEVICE=$(OUT_DIR=$OUT_DIR get_build_var TARGET_DEVICE)

  TITLE="Build CHUB $NANOHUB_TARGET"
  print_title

  CHUB_START_TIME=`date +%s`

  if [[ "$NANOHUB_TARGET" == "" || "$NANOHUB_PATH" == "" ]]
  then
    echo "***********************************************************************"
    echo " >>> warning : nanohub build target or nanohub project path is not specified"
    echo " >>> NANOHUB_TARGET = $NANOHUB_TARGET"
    echo " >>> NANOHUB_PATH = $NANOHUB_PATH"
    echo "***********************************************************************"
    return 1
  fi

  if [ ! -f $NANOHUB_PATH/build.sh ]
  then
    echo "***********************************************************************"
    echo " >>> warning: $NANOHUB_PATH/build.sh does not exist !!!"
    echo "***********************************************************************"
    return 2
  fi

  if [[ $1 == 0 ]]
  then
    export NANOHUB_FIRMWARE_PATH=$ROOT_DIR/device/samsung/$TARGET_DEVICE/firmware
  fi

  echo "*****************************************"
  echo "PATH = $NANOHUB_PATH"
  echo "TARGET = $NANOHUB_TARGET"
  echo "FIRMWARE PATH = $NANOHUB_FIRMWARE_PATH"
  echo "*****************************************"
  export NANOHUB_TOOLCHAIN=$ROOT_DIR/prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-5_3-2016q1/bin/arm-none-eabi-
  export CROSS_COMPILE=$ROOT_DIR/prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-5_3-2016q1/bin/arm-none-eabi-
  export ARM_NONE_GCC_PATH=$ROOT_DIR/prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-5_3-2016q1
  echo "NANOHUB_TOOLCHAIN=$NANOHUB_TOOLCHAIN"
  echo "CROSS_COMPILE=$CROSS_COMPILE"
  echo "ARM_NONE_GCC_PATH=$ARM_NONE_GCC_PATH"
  echo "*****************************************"

  pushd $NANOHUB_PATH
  ./build.sh $NANOHUB_TARGET
  popd

  unset NANOHUB_TOOLCHAIN
  unset CROSS_COMPILE
  unset ARM_NONE_GCC_PATH

  CHUB_END_TIME=`date +%s`
  FUNC=${FUNCNAME[0]}
  let "ELAPSED_TIME=$CHUB_END_TIME-$CHUB_START_TIME"
  echo "${color_success}[BUILD TOOL]${color_reset} $FUNC: ${color_title}$ELAPSED_TIME seconds${color_reset}"
}

function build_android()
{
  if [ "$WITH_ESSI" == "true" ] ; then
    build_essi
  fi
  TITLE="Build android platform $PLATFORM_BUILD_TARGET"
  print_title
  START_TIME=`date +%s`
  echo "source build/envsetup.sh"
  source build/envsetup.sh
  echo
  echo "lunch full_$BUILD_PRODUCT-$BUILD_VARIANT"
  OUT_DIR=$OUT_DIR lunch full_$BUILD_PRODUCT-$BUILD_VARIANT > /dev/null
  check_exit
  build_chub 0
  echo
  echo "make -j$CPU_JOB_NUM $PLATFORM_BUILD_TARGET"
  echo
  MAKE_DIST=$(OUT_DIR=$OUT_DIR get_build_var PRODUCT_USE_DYNAMIC_PARTITIONS)
  if [ "$MAKE_DIST" == "true" ] ; then
    if [ "$PLATFORM_BUILD_TARGET" != "bootimage" ] ; then
      if [ "$WITH_ESSI" != "true" ] ; then
        PLATFORM_BUILD_TARGET=dist
      fi
    fi
  fi
  OUT_DIR=$OUT_DIR make -j$CPU_JOB_NUM $PLATFORM_BUILD_TARGET
  check_exit
  PRODUCT_OUT=$(OUT_DIR=$OUT_DIR get_build_var PRODUCT_OUT)
  if [ "$PLATFORM_BUILD_TARGET" == "bootimage" ] ; then
    cp -f $DIST/* $PRODUCT_OUT/
  fi
  if [ "$PLATFORM_BUILD_TARGET" == "dist" ] ; then
    unzip -o $OUT_DIR/dist/full_$BUILD_PRODUCT-img* -d $PRODUCT_OUT/
    cp -f $OUT_DIR/dist/super.img $PRODUCT_OUT/
  fi
  echo "$PRODUCT_OUT"
  ls -l $PRODUCT_OUT/*.img $PRODUCT_OUT/ff_* 2> /dev/null
  if [ "$WITH_ESSI" == "true" ] ; then
    create_vbmeta
    check_exit
    create_super
    check_exit
  fi
  END_TIME=`date +%s`
  FUNC=${FUNCNAME[0]}
  elapsed_time
}

function audit()
{
  TITLE="launch audit2allow, need audit denined log file name is audit.log and policy file"
  print_title
  START_TIME=`date +%s`
  echo "source build/envsetup.sh"
  source build/envsetup.sh
  echo
  echo "lunch full_$BUILD_PRODUCT-$BUILD_VARIANT"
  OUT_DIR=$OUT_DIR lunch full_$BUILD_PRODUCT-$BUILD_VARIANT &> /dev/null
  cat audit.log | audit2allow -p policy
  check_exit
  END_TIME=`date +%s`
  FUNC=${FUNCNAME[0]}
  elapsed_time
}

function build_all()
{
    START_TIME=`date +%s`
    clean_android "_${BUILD_PRODUCT}_${BUILD_VARIANT}"
    $BUILDSH $BUILD_PRODUCT $BUILD_VARIANT platform $TARGET_BUILD_CARRIER
    check_exit
    END_TIME=`date +%s`
    FUNC="Total"
    elapsed_time
}

export TARGET_BUILD_CARRIER=
if [ $(array_contains $BUILD_OPTION2 "${build_carrier[@]}") == 0 ]; then
    export TARGET_BUILD_CARRIER=$BUILD_OPTION2
    echo "1.TARGET_BUILD_CARRIER is $BUILD_OPTION2: $TARGET_BUILD_CARRIER"
fi

####### Main Routines #######
case "$BUILD_OPTION" in
  platform)
    build_android
    echo "TARGET_BUILD_CARRIER is $TARGET_BUILD_CARRIER"
    ;;
  host-tools)
    build_host_tools
    ;;
  bootimage)
    START_TIME=`date +%s`
    clean_android "_${BUILD_PRODUCT}_${BUILD_VARIANT}"
    PLATFORM_BUILD_TARGET=bootimage $BUILDSH $BUILD_PRODUCT $BUILD_VARIANT platform
    END_TIME=`date +%s`
    FUNC="Total"
    elapsed_time
    ;;
  audit)
    audit
    ;;
  chub)
    build_chub 1
    ;;
#### Work as BUILD_CARRIER ####
  "")
    build_all
    ;;
  *)
    if [ $(array_contains $BUILD_OPTION "${build_carrier[@]}") == 0 ]; then
        export TARGET_BUILD_CARRIER=$BUILD_OPTION
        echo "2.TARGET_BUILD_CARRIER is $BUILD_OPTION : $TARGET_BUILD_CARRIER"
        build_all
        exit 0
    fi
    print_usage_exit "$BUILD_OPTION is not vaild build option"
esac

exit 0
