#!/bin/sh

echo "system update start..."

echo "USB check.."
if [  -b "/dev/sda" ]; then
echo "USB connect"
else 
echo "find no USB"
exit 4
fi



echo "enter recovery start..."
sleep 5
cd /home/app/USB/OTA
chmod 777 setRecoveryFlag
./setRecoveryFlag
reboot
exit 2
