#/bin/bash
# collect throughput, temperature and CPU stats and
# output in CSV format

NUM_SAMPLES=60

count=1
while [ $count -le $NUM_SAMPLES ]
do

## read throughput
TX_TPUT=0
RX_TPUT=0
read if tx tx_unit rx rx_unit < /proc/driver/unifi0/tput
IFS=':' read dummy PROC_TX_TPUT <<< "$tx"
IFS=':' read dummy PROC_RX_TPUT <<< "$rx"
if [ $tx_unit = "Mbps" ]
        then
        TX_TPUT=$(( PROC_TX_TPUT * 1000 * 1000 ))
elif [ $tx_unit = "Kbps" ]
        then
        TX_TPUT=$(( PROC_TX_TPUT * 1000 ))
else
        TX_TPUT=$(( PROC_TX_TPUT ))
fi
if [ $rx_unit = "Mbps" ]
        then
        RX_TPUT=$(( PROC_RX_TPUT * 1000 * 1000 ))
elif [ $rx_unit = "Kbps" ]
        then
        RX_TPUT=$(( PROC_RX_TPUT * 1000 ))
else
        RX_TPUT=$(( PROC_RX_TPUT ))
fi

## read number of interrupts per CPU
INTR=($(sed -n '/scsc_wlbt/p' /proc/interrupts))

## read CPU usage in Firmware
MIB=$(slsi_wlan_mib --vif 1 2254.23)
while read -r line; do
    IFS='=' read dummy FW_CPU <<< "$line"
done <<< "$MIB"

## read temperature
read TEMP_BIG < /sys/class/thermal/thermal_zone0/temp
read TEMP_LITTLE < /sys/class/thermal/thermal_zone1/temp

## read CPU usage from /proc/stat
C0=($(sed '2q;d' /proc/stat))
C1=($(sed '3q;d' /proc/stat))
C2=($(sed '4q;d' /proc/stat))
C3=($(sed '5q;d' /proc/stat))
C4=($(sed '6q;d' /proc/stat))
C5=($(sed '7q;d' /proc/stat))
C6=($(sed '8q;d' /proc/stat))
C7=($(sed '9q;d' /proc/stat))

# 1: user, 2: nice, 3: system, 4: idle, 5: iowait, 6: irq, 7: softirq, 8: steal, 9: guest, 10: guest_nice
# Note: at the moment it is not capturing usage in steal, guest and guest_nice mode
TC0=$((${C0[1]} + ${C0[2]} + ${C0[3]} + ${C0[4]} + ${C0[5]} + ${C0[6]} + ${C0[7]}))
TC1=$((${C1[1]} + ${C1[2]} + ${C1[3]} + ${C1[4]} + ${C1[5]} + ${C1[6]} + ${C1[7]}))
TC2=$((${C2[1]} + ${C2[2]} + ${C2[3]} + ${C2[4]} + ${C2[5]} + ${C2[6]} + ${C2[7]}))
TC3=$((${C3[1]} + ${C3[2]} + ${C3[3]} + ${C3[4]} + ${C3[5]} + ${C3[6]} + ${C3[7]}))
TC4=$((${C4[1]} + ${C4[2]} + ${C4[3]} + ${C4[4]} + ${C4[5]} + ${C4[6]} + ${C4[7]}))
TC5=$((${C5[1]} + ${C5[2]} + ${C5[3]} + ${C5[4]} + ${C5[5]} + ${C5[6]} + ${C5[7]}))
TC6=$((${C6[1]} + ${C6[2]} + ${C6[3]} + ${C6[4]} + ${C6[5]} + ${C6[6]} + ${C6[7]}))
TC7=$((${C7[1]} + ${C7[2]} + ${C7[3]} + ${C7[4]} + ${C7[5]} + ${C7[6]} + ${C7[7]}))

## read CPU frequency
F0=($(sed -n '1,12p' /sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state))
F4=($(sed -n '1,14p' /sys/devices/system/cpu/cpu4/cpufreq/stats/time_in_state))

echo "$count,$TX_TPUT,$RX_TPUT,$TEMP_BIG,$TEMP_LITTLE,$FW_CPU,${INTR[1]},${INTR[2]},${INTR[3]},${INTR[4]},${INTR[5]},${INTR[6]},${INTR[7]},${INTR[8]},,${C0[3]},${C0[7]},${C0[4]},$TC0,${C1[3]},${C1[7]},${C1[4]},$TC1,${C2[3]},${C2[7]},${C2[4]},$TC2,${C3[3]},${C3[7]},${C3[4]},$TC3,${C4[3]},${C4[7]},${C4[4]},$TC4,${C5[3]},${C5[7]},${C5[4]},$TC5,${C6[3]},${C6[7]},${C6[4]},$TC6,${C7[3]},${C7[7]},${C7[4]},$TC7,,${F0[0]},${F0[1]},${F0[2]},${F0[3]},${F0[4]},${F0[5]},${F0[6]},${F0[7]},${F0[8]},${F0[9]},${F0[10]},${F0[11]},${F0[12]},${F0[13]},${F0[14]},${F0[15]},${F0[16]},${F0[17]},${F0[18]},${F0[19]},${F0[20]},${F0[21]},${F0[22]},${F0[23]},${F4[0]},${F4[1]},${F4[2]},${F4[3]},${F4[4]},${F4[5]},${F4[6]},${F4[7]},${F4[8]},${F4[9]},${F4[10]},${F4[11]},${F4[12]},${F4[13]},${F4[14]},${F4[15]},${F4[16]},${F4[17]},${F4[18]},${F4[19]},${F4[20]},${F4[21]},${F4[22]},${F4[23]},${F4[24]},${F4[25]},${F4[26]},${F4[27]}"

(( count++ ))
sleep .8
done

