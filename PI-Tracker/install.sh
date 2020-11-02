#!/bin/bash
BASEPATH="$(dirname "$(readlink -f "$0")")"

ln -sf "$BASEPATH/htracker@.service" /etc/systemd/system/
ln -sf "$BASEPATH/htracker" /usr/local/bin/

FILE=/boot/starter.txt
if test -f "$FILE"; then
echo "## [htracker] M5-CAN-Tracker lidar
# htracker@ok
" >> /boot/starter.txt
fi