#!/system/bin/sh
# NS-IOT PortFwd Script V0.8
# Change History
# V0.8 : Support Android O version, fixed clatif indexing
# V0.71: Bugfix for invalid index for interfacename array
# V0.7 : Use mapping array for ipaddr to distinguish interface, and add verbose option for iptables
# V0.6 : Fix PC side IPv4 address fetching. Added '-4' optoin to 'ip neigh'
# V0.5 : Add CLAT IPV4 interface port forwarding setting
# V0.4 : Improve IP neigh stability corresponding to Tethering Reconnection, added -w option
# V0.3 : Support M version IPv4 mobile network, added some ipv6 mobile network, but not work
# V0.2 : Added UDP/TCP Port Override, Added SYN Packet Drop rule option
# This will set iptable nat table for Portforwarding rule
# to test NS IOT
# all input from up interface of rmnet? will be forwarded to pc ip address through rndis
# Constraint : This will only work for IPv4 Mobile Network to IPv4 Tethered Network

pcaddr=$(getprop vendor.net.nstest_pcaddr)
scriptver=0.8
tcpport=5011
udpport=5013
dummy="jiwon"
sourceif="rmnet"
targetif="rndis"
logout="/data/vendor/testscript/runresult"
logout_OLD="/data/vendor/testscript/runresult_old"

# preserve old log fully
#if [ -e $logout ];
#then
#    grep "###" $logout > $logout_OLD
#    mv $logout_OLD $logout
#fi;
echo "=================================================" >> $logout

echo "### V$scriptver Start Test port forwarding @ $(date)" >> $logout
chown root.shell $logout
chmod 640 $logout


# Check Port Value Overriding
ov_udpport=$(getprop vendor.config.net.nstest_udpport)
ov_tcpport=$(getprop vendor.config.net.nstest_tcpport)

if [ ! $ov_udpport$dummy == "0"$dummy ] && [ ! $ov_udpport$dummy == ""$dummy ]
then
    echo Set udp port to $ov_udpport \(Override\) >> $logout;
    udpport=$ov_udpport
else
    echo Set udp port to $udpport \(Default\) >> $logout;
fi;

if [ ! $ov_tcpport$dummy == "0"$dummy ] && [ ! $ov_tcpport$dummy == ""$dummy ]
then
    echo Set tcp port to $ov_tcpport \(Override\) >> $logout;
    tcpport=$ov_tcpport
else
    echo Set tcp port to $tcpport \(Default\) >> $logout;
fi;

#Check SYN Packet Drop Option Rule
synpacketdrop=$(getprop vendor.config.net.nstest_synpacketdrop)

# Search IP neighbor of rndis0 only IPV4
ipneigh=( $(ip -4 neigh) )
timeout=2
# TBD: This loop makes data connection delay.
while test $timeout -gt 0
do
	if [ $timeout -gt 0 ]
    then
        sleep 1
    else
		echo out loop by timeout >> $logout;
        break;
	fi
    if [ $dummy$ipneigh == $dummy"" ]
    then
        ipneigh=( $(ip -4 neigh) );
        (( timeout-- ));
        echo ip neigh = ${ipneigh[@]} , timeout=$timeout >> $logout;
    else
        echo found ip neigh, exit loop >> $logout;
        break;
    fi
done

#neighlist=`echo $ipneigh | grep -oE "[^ ]+"`
echo "Result of # ip neigh" >> $logout
ipneigh=( $(ip -4 neigh) );
echo ${ipneigh[@]} >> $logout

count=0
if [ $pcaddr$dummy == ""$dummy ]
then
    for elm in "${ipneigh[@]}";
    do
        if [[ ${ipneigh[$count]} == $targetif* ]]
        then
# ipv4 will be listed below, so we will catch last rndis entry
        pcaddr=${ipneigh[$count-2]}
        echo "Found PC's ip addr: $pcaddr" >> $logout;
        fi;
        (( count++ ))
    done
else
    echo "use pcaddr from vendor.net.nstest_pcaddr" >> $logout
fi

if [ $pcaddr$dummy == ""$dummy ]
then
  echo "### ERROR: PC's ip addr is NOT found, so iptable rules are not changed\n" >> $logout;
# Check iptables result
    echo "# # ERROR: Original iptable result of nat table:" >> $logout
    iptables -w -t nat -L -v -n >> $logout
    echo "# # ERROR: Original iptable result of filter table:" >> $logout
    iptables -w -L -v -n >> $logout
    echo "# # ERROR: Original iptable result of mangle table:" >> $logout
    iptables -w -t mangle -L -v -n >> $logout
  echo "\n# # ERROR: PC's ip addr is NOT found, so iptable rules are not changed" >> $logout;
  if [ $timeout -eq 0 ]
  then
      echo "### ERROR: IP neigh command go over timeout, It's recommended to reboot UE" >> $logout;
  fi
  exit 3
fi;

# flush netfilter filter table rules
osversion=$(getprop ro.build.version.release)
echo "### Checked Android Version : $osversion" >> $logout

oIFS="$IFS"
IFS=.
set -- $osversion
IFS="$oIFS"

ver_major=$1
ver_minor=$2
ver_patch=$3
echo OS version major=$ver_major, minor=$ver_minor, patch=$ver_patch >> $logout

if [ $ver_major -ge 6 ] || [ $ver_major == 'N' ] || [ $ver_major == 'M' ] || [ $ver_major == 'O' ]
then
echo "Android vesion is greater than 6(M)" >> $logout

# new ip addr style this can be return two more address
mobileifname=( $(ip addr show up label *$sourceif* | grep -oE "[0-9*: $sourceif[0-9]*" | grep -oE "$sourceif[0-9]*") )
clatifname=( $(ip addr show up label *$sourceif* | grep -oE "[0-9]*: [0-9a-z\-]+$sourceif[0-9]*" | grep -oE "[0-9a-z\-]+$sourceif[0-9]*") )

# Not used, back quote style
#zipaddr=`ip addr show up label *$sourceif* | grep "inet " | grep -E "[0-9./]+"`
#zipaddr6=`ip addr show up label *$sourceif* | grep "inet6 " | grep -oE "[0-9a-z:/]+"`
#`echo $zipaddr6 | grep -oE "[0-9./]+"` >> $logout

# new ip address table for each rmnet number
if_count=0
clat_if_count=0
for elm in "${mobileifname[@]}";
do
    echo elm=$elm >> $logout
    ifidx=$( echo $elm | grep -o "[0-9]" )
    temp=( $(ip addr show up label $elm | grep -oE "inet ([0-9./]+)" | grep -oE "[0-9./]+") )
    ipa[$ifidx]=$temp
    (( if_count++ ))
done

# new ip address table for each v4-rmnet number
for elm in "${clatifname[@]}";
do
    echo elm=$elm >> $logout
    ifidx=$( echo $elm | grep -o "[0-9]$" )
    echo ifindex=$ifidx >> $logout
    temp=( $(ip addr show up label $elm | grep -oE "inet ([0-9./]+)" | grep -oE "[0-9./]+") )
    clat_ipa[$ifidx]=$temp
    (( clat_if_count++ ))
done

# dump searching result
echo clatif=${clatifname[@]} >> $logout
echo mobileif=${mobileifname[@]} >> $logout
echo "Result of ip addr show up label *$sourceif*" >> $logout
echo "IpV4 Result" >> $logout

function dump_ipaddr() {
for elm in "${mobileifname[@]}";
do
    ifidx=$( echo $elm | grep -o "[0-9]" )
    echo from $elm with $ifidx ipv4 address is >> $logout
    echo ${ipa[$ifidx]} >> $logout
done

for elm in "${clatifname[@]}";
do
    ifidx=$( echo $elm | grep -o "[0-9]$" )
    echo from $elm with $ifidx ipv4 address is >> $logout
    echo ${clat_ipa[$ifidx]} >> $logout
done
}

# RUN: dump_ipaddr
dump_ipaddr


# Not used
function old_get_ipv4_addr() {
ipv4addr_count=0
count=0
ipaddr=( $(ip addr show up label $sourceif* | grep "inet " | grep -E "[0-9./]+") )
for elm in "${ipaddr[@]}"
do
#echo "ipaddr[$count]="${ipaddr[$count]} >> $logout;
   if [ $elm == "inet" ]
   then
      ipv4addr[$ipv4addr_count]=${ipaddr[$count+1]};
      echo ipv4addr=${ipv4addr[$ipv4addr_count]} >> $logout
      (( ipv4addr_count++ ))
   fi
# we found up inet address for rmnet
   (( count++ ))
done
}

# Not used
function old_get_clat_ipv4_addr() {
clatipv4addr_count=0
count=0
clatipaddr=( $(ip addr show up label v4-$sourceif* | grep "inet " | grep -E "[0-9./]+") )
for elm in "${clatipaddr[@]}";
do
#echo "ipaddr[$count]="${ipaddr[$count]} >> $logout;
   if [ $elm == "inet" ]
   then
      clatipv4addr[$clatipv4addr_count]=${clatipaddr[$count+1]};
      echo clatipv4addr=${clatipv4addr[$clatipv4addr_count]} >> $logout
      (( clatipv4addr_count++ ))
   fi
# we found up inet address for rmnet
    (( count++ ))
done
}

# Not used
function get_ipv6_addr() {
ipv6addr_count=0
count=0
ipaddr6=( $(ip addr show up label $sourceif* | grep "inet6 " | grep -oE "[0-9a-z:/]+") )
echo "IpV6 Result" >> $logout
for elm in "${ipaddr6[@]}";
do
    if [ $elm == "inet6" ]
    then
        ipv6addr[$ipv6addr_count]=${ipaddr6[$count+1]};
        echo ipv6addr=${ipv6addr[$ipv6addr_count]} >> $logout
        (( ipv6addr_count++ ))
    fi
    (( count++ ))
done
}


function flush_iptables() {
# flush netfilter filter table rules
echo "applying # iptables -w -F" >> $logout
iptables -w -F
# flush netfilter nat table rules
echo "applying # iptables -w -t nat -F" >> $logout
iptables -w -t nat -F
}

# RUN: Flush iptables tables
flush_iptables

# Not used
function old_set_iptables_for_ipv4addr() {
count=0
for elm in "${ipv4addr[@]}";
do
    ipaddr=${ipv4addr[$count]};
    ifname=${mobileifname[$count]};
    echo "for $ifname all input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "for $ifname all udp input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport" >> $logout
    iptables -w -t nat -I PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport >> $logout
    echo "for $ifname all tcp input will go through $ipaddr:$tcpport to $pcaddr:$tcpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport" >> $logout
    iptables -w -t nat -I PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport >> $logout
    echo "for $ifname all output will has same src $ipaddr from anywhere\n" >> $logout
echo "applying # iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE" >> $logout
    iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE >> $logout
    (( count++ ))
done
count=0
for elm in "${clatipv4addr[@]}";
do
    ipaddr=${clatipv4addr[$count]};
    ifname=${clatifname[$count]};
    echo "for $ifname all input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "for $ifname all udp input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport" >> $logout
    iptables -w -t nat -I PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport >> $logout
    echo "for $ifname all tcp input will go through $ipaddr:$tcpport to $pcaddr:$tcpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport" >> $logout
    iptables -w -t nat -I PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport >> $logout
    echo "for $ifname all output will has same src $ipaddr from anywhere\n" >> $logout
    echo "applying # iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE" >> $logout
    iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE >> $logout
    (( count++ ))
done
}

function set_iptables_for_ipv4addr() {
for elm in "${mobileifname[@]}";
do
    ifidx=$( echo $elm | grep -o "[0-9]" )
    ipaddr=${ipa[$ifidx]};
    if [ $ipaddr$dummy == ""$dummy ]; then
        continue;
    fi;
    ifname=$elm;
    echo "for $ifname all input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "for $ifname all udp input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport" >> $logout
    iptables -w -t nat -I PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport >> $logout
    echo "for $ifname all tcp input will go through $ipaddr:$tcpport to $pcaddr:$tcpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport" >> $logout
    iptables -w -t nat -I PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport >> $logout
    echo "for $ifname all output will has same src $ipaddr from anywhere\n" >> $logout
echo "applying # iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE" >> $logout
    iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE >> $logout
done
for elm in "${clatifname[@]}";
do
    ifidx=$( echo $elm | grep -o "[0-9]$" )
    ipaddr=${clat_ipa[$ifidx]};
    if [ $ipaddr$dummy == ""$dummy ]; then
        continue;
    fi;
    ifname=$elm;
    echo "for $ifname all input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "for $ifname all udp input will go through $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport" >> $logout
    iptables -w -t nat -I PREROUTING -p udp -i $ifname -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport >> $logout
    echo "for $ifname all tcp input will go through $ipaddr:$tcpport to $pcaddr:$tcpport" >> $logout
    echo "applying # iptables -w -t nat -A PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport" >> $logout
    iptables -w -t nat -I PREROUTING -p tcp -i $ifname -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport >> $logout
    echo "for $ifname all output will has same src $ipaddr from anywhere\n" >> $logout
    echo "applying # iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE" >> $logout
    iptables -w -t nat -A POSTROUTING -o $ifname -j MASQUERADE >> $logout
done
}

# RUN: set iptables for portforwarding
set_iptables_for_ipv4addr


# for unknown SYN PACKET DROP
if [ ! $synpacketdrop$dummy == "0"$dummy ] && [ ! $synpacketdrop$dummy == ""$dummy ]
then
    echo "applying SYN Packet Drop rules by Option enables" >> $logout
    echo "applying # iptables -w -A INPUT ! -d 192.0.0.0/8 -j DROP" >> $logout
    iptables -w -A INPUT ! -d 192.0.0.0/8 -j DROP >> $logout
fi;
changed=1


# End if
else

# old netcfg style
netcfginfo=( $(netcfg) )

count=0
line=""
echo "Result of netcfg" >> $logout
for elm in "${netcfginfo[@]}";
do
   if (( count != 0 )) && (( (( count % 5 )) == 0 )) then echo $line"">> $logout; line=""; fi;
   line=$line" "${netcfginfo[$count]};
   (( count++ ))
done
echo "" >> $logout

count=0
changed=0

for elm in "${netcfginfo[@]}";
do
   if [ $elm == "UP" ]
   then
      if [[ ${netcfginfo[$count-1]} == $sourceif* ]]
      then
# flush netfilter filter table rules
        echo "applying # iptables -w -F" >> $logout
        iptables -w -F
# flush netfilter nat table rules
        echo "applying # iptables -w -t nat -F" >> $logout
        iptables -w -t nat -F

         ifnum=`echo ${netcfginfo[$count-1]} | grep -oE "[^rmnet]+"`
         echo $count ${netcfginfo[$count-1]} $ifnum $elm ${netcfginfo[$count+1]};
        rmnet=${netcfginfo[$count-1]}
        ipaddr=${netcfginfo[$count+1]}

        echo "for $rmnet all input will go from $ipaddr:$udpport to $pcaddr:$udpport\n" >> $logout
        echo "for $rmnet all udp input will go from $ipaddr:$udpport to $pcaddr:$udpport" >> $logout
        echo "applying # iptables -w -t nat -A PREROUTING -p udp -i $rmnet -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport" >> $logout
        iptables -w -t nat -A PREROUTING -p udp -i $rmnet -d $ipaddr --dport $udpport -j DNAT --to $pcaddr:$udpport >> $logout
        echo "for $rmnet all tcp input will go from $ipaddr:$tcpport to $pcaddr:$tcpport" >> $logout
        echo "applying # iptables -w -t nat -A PREROUTING -p tcp -i $rmnet -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport" >> $logout
        iptables -w -t nat -A PREROUTING -p tcp -i $rmnet -d $ipaddr --dport $tcpport -j DNAT --to $pcaddr:$tcpport >> $logout
        echo "for $rmnet all output will has same src $ipaddr from anywhere\n" >> $logout
        echo "applying # iptables -w -t nat -A POSTROUTING -o $rmnet -j MASQUERADE" >> $logout
        iptables -w -t nat -A POSTROUTING -o $rmnet -j MASQUERADE >> $logout
# for unknown SYN PACKET DROP
        if [ ! $synpacketdrop$dummy == "0"$dummy ] && [ ! $synpacketdrop$dummy == ""$dummy ]
        then
            echo "applying SYN Packet Drop rules by Option enables" >> $logout
            echo "applying # iptables -w -A INPUT ! -d 192.0.0.0/8 -j DROP" >> $logout
            iptables -w -A INPUT ! -d 192.0.0.0/8 -j DROP >> $logout
        fi;
        changed=1
     fi;
   fi;
   (( count++ ))
done

fi;

#check if there is changing in iptables
if (( changed == 0 )) then
    echo "### ERROR: Mobile Network Interface is not UP state, so iptable rules are not changed" >> $logout;
    echo "# # ERROR: Original iptable result of nat table:" >> $logout
    iptables -w -t nat -L -v -n >> $logout
    echo "# # ERROR: Original iptable result of filter table:" >> $logout
    iptables -w -L -v -n >> $logout
    echo "# # ERROR: Original iptable result of mangle table:" >> $logout
    iptables -w -t mangle -L -v -n >> $logout
    echo "\n# # ERROR: Mobile Network Interface is not UP state, so iptable rules are not changed" >> $logout;
exit 3
fi

# Check iptables result
echo "!iptable result of nat table:" >> $logout
iptables -w -t nat -L -v -n >> $logout
echo "!iptable result of filter table:" >> $logout
iptables -w -L -v -n >> $logout
echo "!iptable result of mangle table:" >> $logout
iptables -w -t mangle -L -v -n >> $logout
# Rechecking is useless with '-w' option and hazard for delay, so deleted

if [ $? -eq 3 ]; 
  then
     echo "### SOMETHING WRONG @ $(date)" >> $logout;
  elif [ $? -eq 2  ] || [ $? -eq 1 ];
  then
     echo "### SETTING IS OK @ $(date)" >> $logout;
  else 
     echo "### RESULT:$?  @ $(date)" >> $logout;
fi
