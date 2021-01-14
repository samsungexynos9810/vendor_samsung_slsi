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
  echo 'Usage: ./build.sh <PRODUCT> [ eng | userdebug | user ] [ "" | bootimage | host-tools | product | platform ] [ <TARGET_BUILD_CARRIER> ]'
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

if [ -z $EXYNOS_PRODUCT_NAME ]; then
  EXYNOS_PRODUCT_NAME="product_"$BUILD_PRODUCT
fi

#PRODUCT_MANIFEST=$ROOT_DIR/product
if [ ! -d "$ROOT_DIR/$EXYNOS_PRODUCT_NAME" ]
then
EXYNOS_PRODUCT_NAME="product"
fi

PRODUCT_MANIFEST=$ROOT_DIR/$EXYNOS_PRODUCT_NAME
TITLE="Exynos Product Folder Name is $PRODUCT_MANIFEST"
print_title

TARGET_KERNEL=$PRODUCT_MANIFEST/kernel
PRODUCT_TOOLCHAIN_BASE=$PRODUCT_MANIFEST/toolchain

KERNEL_CONFIG=$TARGET_KERNEL/build.config.$BUILD_PRODUCT
SUBCONFIG_BASE=$PRODUCT_MANIFEST/script/configs
KERNEL_SUBCONFIG=$SUBCONFIG_BASE/kernel.build.config.$BUILD_PRODUCT

KERNEL_CLANG_BASE=$ROOT_DIR/prebuilts/clang/host/linux-x86
KERNEL_GCC_PATH=$ROOT_DIR/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
KERNEL_MERGE_CONFIG=$TARGET_KERNEL/scripts/kconfig/merge_config.sh
MKDTIMG=$ROOT_DIR/host_tools/host/linux-x86/bin/mkdtimg
LPMAKE=$ROOT_DIR/host_tools/host/linux-x86/bin/lpmake
UFDT_APPLY_OVERLAY=$ROOT_DIR/host_tools/host/linux-x86/bin/ufdt_apply_overlay
BUILD_SUPER_IMAGE=$ROOT_DIR/build/make/tools/releasetools/build_super_image.py

DIST=$ROOT_DIR/device/samsung/${BUILD_PRODUCT}-prebuilts
PRODUCT_DIST=$PRODUCT_MANIFEST/out_$BUILD_PRODUCT

####### Clean up Sequences #######
function clean_dist()
{
  TITLE="Clean Main DIST"
  print_title
  echo "rm -rf $DIST"
  rm -rf $DIST
  mkdir -p $DIST
}

function clean_kernel()
{
  TITLE="Clean kernel"
  print_title
  PATH=$KERNEL_GCC_PATH:$PATH
  CROSS_COMPILE=aarch64-linux-android-
  echo "make -C $TARGET_KERNEL -j distclean"
  make -C $TARGET_KERNEL -j distclean
}

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
  clean_dist
  clean_kernel
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

function check_kernel()
{
  if [ ! -d $TARGET_KERNEL ]; then
    print_error "$TARGET_KERNEL: No kernel source"
    exit 2
  fi

  if [ ! -f $KERNEL_CONFIG ]; then
    if [ -f $KERNEL_SUBCONFIG ]; then
      KERNEL_CONFIG=$KERNEL_SUBCONFIG
    else
      print_error "$KERNEL_CONFIG: No kernel config"
      print_error "$KERNEL_SUBCONFIG: No kernel subconfig"
      exit 2
    fi
  fi
}

function check_environment()
{
  if [[ ! -z "${TARGET_PRODUCT}" ]]; then
    print_error "\"build/envsetup.sh\" and \"lunch\" should not be executed. Please re-create your shell"
    exit 2
  fi
}

check_kernel
check_environment
$PRODUCT_MANIFEST/prod_build.sh $BUILD_PRODUCT check
check_exit

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

function copy_misc_bin()
{
  TITLE="Copy Product DIST to Main DIST "
  print_title
  echo
  echo $DIST
  ls -la $DIST
}

####### Build Sequencess #######
function build_defconfig()
{
  TITLE="Build kernel(1) defconfig"
  print_title
  KERNEL_DEFCONFIG_PATH=$KERNEL_CONFIG_BASE/$DEFCONFIG
  KERNEL_DEFCONFIG_BASE=$KERNEL_CONFIG_BASE/${TARGET_SOC}-base_defconfig
  if [ ! -f $KERNEL_DEFCONFIG_BASE ]; then
    KERNEL_DEFCONFIG_BASE=""
  fi
  if [ "eng" == "$BUILD_VARIANT" ]; then
    KERNEL_USER_CFG=""
  elif [ "userdebug" == "$BUILD_VARIANT" ]; then
    KERNEL_USER_CFG=$KERNEL_CONFIG_BASE/${TARGET_SOC}_userdebug.cfg
  elif [ "user" == "$BUILD_VARIANT" ]; then
    KERNEL_USER_CFG=$KERNEL_CONFIG_BASE/${TARGET_SOC}_user.cfg
  fi

  if [ ! -z ${SEPERATE_KERNEL_OBJ} ]; then
    MAKE_CONFIG_CMD="$KERNEL_MERGE_CONFIG -m -O $TARGET_KERNEL $KERNEL_DEFCONFIG_PATH $KERNEL_DEFCONFIG_BASE $KERNEL_USER_CFG &> /dev/null;"
    MAKE_CONFIG_CMD="$MAKE_CONFIG_CMD make -C $TARGET_KERNEL O=$KERNEL_OBJ KCONFIG_ALLCONFIG=.config alldefconfig -j$CPU_JOB_NUM $CC_CLANG;"
    MAKE_CONFIG_CMD="$MAKE_CONFIG_CMD make -C $TARGET_KERNEL distclean -j$CPU_JOB_NUM $CC_CLANG;"
  else
    MAKE_CONFIG_CMD="$KERNEL_MERGE_CONFIG -m -O $TARGET_KERNEL $KERNEL_DEFCONFIG_PATH $KERNEL_DEFCONFIG_BASE $KERNEL_USER_CFG &> /dev/null;"
    MAKE_CONFIG_CMD="$MAKE_CONFIG_CMD make -C $TARGET_KERNEL KCONFIG_ALLCONFIG=.config alldefconfig -j$CPU_JOB_NUM $CC_CLANG;"
  fi

  echo "$MAKE_CONFIG_CMD"
  bash -c "$MAKE_CONFIG_CMD"
  check_exit
  echo
}

function build_kernel_core()
{
  TITLE="Build kernel(2)"
  print_title
  if [ ! -z ${SEPERATE_KERNEL_OBJ} ]; then
    echo "make -C $TARGET_KERNEL -j$CPU_JOB_NUM O=$KERNEL_OBJ $CC_CLANG"
    make -C $TARGET_KERNEL -j$CPU_JOB_NUM O=$KERNEL_OBJ $CC_CLANG
  else
    echo "make -C $TARGET_KERNEL -j$CPU_JOB_NUM $CC_CLANG"
    make -C $TARGET_KERNEL -j$CPU_JOB_NUM $CC_CLANG
  fi
  check_exit
  echo
}

function build_dtbo()
{
  TITLE="Build kernel(3) dtbo"
  print_title
  if [ "None" == "${TARGET_DTBO_CFG}" ]; then
    return 0;
  fi
  if [ ! -z ${TARGET_DTBO_CFG} ]; then
    KERNEL_DTBO_CFG=$KERNEL_DTB_SOURCE_DIR/${TARGET_DTBO_CFG}
  else
    KERNEL_DTBO_CFG=$KERNEL_DTB_SOURCE_DIR/${TARGET_SOC}_dtboimg.cfg
  fi
  if [ ! -z ${SEPERATE_KERNEL_OBJ} ]; then
    echo "$MKDTIMG cfg_create $KERNEL_OBJ/dtbo.img $KERNEL_DTBO_CFG -d $KERNEL_OBJ"
    $MKDTIMG cfg_create $KERNEL_OBJ/dtbo.img $KERNEL_DTBO_CFG -d $KERNEL_OBJ
  else
    echo "$MKDTIMG cfg_create $TARGET_KERNEL/dtbo.img $KERNEL_DTBO_CFG -d $TARGET_KERNEL"
    $MKDTIMG cfg_create $TARGET_KERNEL/dtbo.img $KERNEL_DTBO_CFG -d $TARGET_KERNEL
  fi
  check_exit
  echo
}

function check_dtbo_merge()
{
  TITLE="Build kernel(3-1) dtbo merge verification"
  print_title
  ## find dtb/dtbo file name in kernel build.config ##
  IFS=' ' read -ra BINARR <<< $(echo "$BINLIST" | tr "\n" " ")
  for bin in "${BINARR[@]}"; do
    IFS=':' read -ra split <<< $bin
    if [ ! -f $BINBASE/${split[0]} ]; then
      print_error "$BINBASE/${split[0]}: No such file"
      exit 2
    fi
    if [ "dtb.img" == "${split[1]}" ]; then
      KERNEL_DTB=$BINBASE/${split[0]}
    fi
    if [ "dtbo.img" == "${split[1]}" ]; then
      KERNEL_DTBO=$BINBASE/${split[0]}
    fi
  done

  ## error check ##
  if [ -z ${KERNEL_DTB} ]; then
      print_error "ERROR: Can not find dtb file name in build.config"
      exit 2
  fi

  if [ -z ${KERNEL_DTBO} ]; then
      print_error "ERROR: Can not find dtbo file name in build.config"
      exit 2
  fi

  ## delete pre dtbo_dump.* files ##
  rm -f $BINBASE/dtbo_dump.*

  ## dump dtbo entry files from dtbo.img ##
  echo "$MKDTIMG dump $KERNEL_DTBO -b $BINBASE/dtbo_dump"
  $MKDTIMG dump $KERNEL_DTBO -b $BINBASE/dtbo_dump > /dev/null
  check_exit

  ## verify to merge with dtb and dtbo ##
  local merge_ret
  local dtbo
  for dtbo in $(ls $BINBASE/dtbo_dump.*)
  do
    echo "$dtbo"
    merge_ret=$($UFDT_APPLY_OVERLAY $KERNEL_DTB $dtbo $dtbo.out 2>&1)
    echo  $merge_ret
    if [[ $merge_ret =~ "ERROR" ]]; then
      exit 2
    fi
  done

  ## clean dtbo_dump.* files ##
  rm -f $BINBASE/dtbo_dump.*
}

function build_kernel()
{
  check_kernel
  START_TIME=`date +%s`
  unset $(cat $KERNEL_CONFIG | grep -v FILES | grep -v "[:\"]" | cut -d= -f1)
  export $(cat $KERNEL_CONFIG | grep -v FILES | grep -v "[:\"]" | cut -d= -f1)
  source $KERNEL_CONFIG
  KERNEL_CONFIG_BASE=$TARGET_KERNEL/arch/$ARCH/configs
  KERNEL_DTB_SOURCE_DIR=$TARGET_KERNEL/arch/$ARCH/boot/dts/exynos
  if [ ! -z ${SEPERATE_KERNEL_OBJ} ]; then
    KERNEL_OBJ=$PRODUCT_MANIFEST/KERNEL_OBJ
  fi
  echo
  PATH=$KERNEL_GCC_PATH:$PATH
  if [ ! -z ${CLANG_VERSION} ]; then
    CC_CLANG="CC=clang"
    if [ ! "z${KERNEL_LTO_ON_4_19}" == "z" ] ; then
      CC_CLANG="CC=clang LD=ld.lld"
    fi
    CLANG_CANDIDATE=$PRODUCT_TOOLCHAIN_BASE/$CLANG_VERSION
    if [ -d ${CLANG_CANDIDATE} ]; then
      PATH=$CLANG_CANDIDATE/bin:$PATH
    else
      CLANG_CANDIDATE=$KERNEL_CLANG_BASE/$CLANG_VERSION
      if [ -d ${CLANG_CANDIDATE} ]; then
        PATH=$CLANG_CANDIDATE/bin:$PATH
      else
        print_error "$PRODUCT_TOOLCHAIN_BASE/$CLANG_VERSION: No clang in product/toolchain"
        print_error "$KERNEL_CLANG_BASE/$CLANG_VERSION: No clang in platform/prebuilts"
        exit -2
      fi
    fi

    if [ ! "z${KERNEL_LTO_ON}" == "z" ] ; then
      export LLVM_AR=$CLANG_CANDIDATE/bin/llvm-ar
      export LLVM_DIS=$CLANG_CANDIDATE/bin/llvm-dis
      export LTO_LLVM_LIB_BASE=$CLANG_CANDIDATE/lib64/
    fi
  fi

  build_defconfig
  build_kernel_core
  build_dtbo
  BINLIST=$FILES
  if [ ! -z ${SEPERATE_KERNEL_OBJ} ]; then
    BINBASE=$KERNEL_OBJ
  else
    BINBASE=$TARGET_KERNEL
  fi
  if [ "z${SKIP_DTBO_MERGE_VERIFICATION}" == "z" ]; then
      check_dtbo_merge
  fi
  copy_binlist
  check_exit
  END_TIME=`date +%s`
  FUNC=${FUNCNAME[0]}
  elapsed_time
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
  # if [ ! -z ${SEPERATE_KERNEL_OBJ} ]; then
  #   TARGET_OUT_INTERMEDIATES=$(OUT_DIR=$OUT_DIR get_build_var TARGET_OUT_INTERMEDIATES)
  #   mv $PRODUCT_MANIFEST/KERNEL_OBJ $TARGET_OUT_INTERMEDIATES
  # fi
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

function build_product()
{
  $PRODUCT_MANIFEST/prod_build.sh $BUILD_PRODUCT
  check_exit
  cp $PRODUCT_DIST/* $DIST
}

function build_all()
{
    START_TIME=`date +%s`
    clean_dist
    clean_android "_${BUILD_PRODUCT}_${BUILD_VARIANT}"
    $BUILDSH $BUILD_PRODUCT $BUILD_VARIANT product
    check_exit
    $BUILDSH $BUILD_PRODUCT $BUILD_VARIANT kernel
    check_exit
    copy_misc_bin
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
  product)
    build_product
    ;;
  kernel)
    $BUILDSH $BUILD_PRODUCT $BUILD_VARIANT host-tools
    build_kernel
    ;;
  bootimage)
    START_TIME=`date +%s`
    clean_dist
    clean_android "_${BUILD_PRODUCT}_${BUILD_VARIANT}"
    $BUILDSH $BUILD_PRODUCT $BUILD_VARIANT kernel
    check_exit
    copy_misc_bin
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
