#/system/bin/sh
# Switch between WLAN production mode and sniffer mode

# Exit on any error
set -e

if [ "$#" -ge 1 ] ; then
	if [ "$1" -eq 0 ] ; then
		echo "Stopping WLAN sniffer, enabling production mode"
		# Stop any existing WLAN mode (belt and braces)
		svc wifi disable
		ifconfig wlan0 down
		ifconfig p2p0 down
                svc wifi enable
	elif [ "$1" -eq 1 ] ; then
		echo "Start WLAN in sniffer mode"
		# Stop any existing WLAN mode (belt and braces)
		svc wifi disable
		ifconfig wlan0 down
		ifconfig p2p0 down
		# Start WLAN without Android framework, in test mode.
		iw wlan0 set monitor none
		ifconfig wlan0 up
		# Set Channel
		if [ "$2" -eq 0 ] ; then
			echo "No channel provided: configure channel Manually"
			echo "Usage: iw wlan0 set freq <freq> [HT20|HT40+|HT40-]"
		else
			iw wlan0 set freq $2 $3 $4
		fi
	else
		echo "Invalid value $1 for input parameter"
	fi
else
	echo "One input parameter must be provided: 1 - sniffer mode, or 0 - production mode"
	echo "Usage: enable_monitor_mode.sh 1 <freq>"
fi
