#!/system/bin/sh
# On/off exerciser via null service

i=0
fail=0
while [ $fail -eq 0 ] ;
do
	echo 1 >/dev/mx_client_test_0
	fail=$?
	# Off always returns OK
	echo 0 >/dev/mx_client_test_0
	i=$((i+1))
done

echo "FAIL $i"
