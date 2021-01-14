#!/vendor/bin/sh
# Dump WLBT-related LDO status
#
# Values are for 7885
#
ADDR=/d/s2mpu08-regs/i2caddr
DATA=/d/s2mpu08-regs/i2cdata

echo "----------------"
echo "WLBT LDO Config:"

echo "----------------"
echo -n "LDO15 "
echo 0x3b > $ADDR
cat $DATA

echo -n "LDO16 "
echo 0x3c > $ADDR
cat $DATA

echo -n "LDO17 "
echo 0x3d > $ADDR
cat $DATA

echo -n "LDO28 "
echo 0x48 > $ADDR
cat $DATA

echo -n "LDO29 "
echo 0x49 > $ADDR
cat $DATA

echo -n "LDO30 "
echo 0x4a > $ADDR
cat $DATA

echo -n "LDO31 "
echo 0x4b > $ADDR
cat $DATA

echo -n "LDO32 "
echo 0x4c > $ADDR
cat $DATA

echo "----------------"

