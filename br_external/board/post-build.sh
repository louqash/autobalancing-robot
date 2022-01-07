#!/bin/bash

set -u
set -e

# Add a console on tty1
if [ -e ${TARGET_DIR}/etc/inittab ]; then
    grep -qE '^tty1::' ${TARGET_DIR}/etc/inittab || \
	sed -i '/GENERIC_SERIAL/a\
tty1::respawn:/sbin/getty -L  tty1 0 vt100 # HDMI console' ${TARGET_DIR}/etc/inittab
fi

HERE=$(dirname $0)

cp ${HERE}/interfaces ${TARGET_DIR}/etc/network/interfaces
cp ${HERE}/wpa_supplicant.conf ${TARGET_DIR}/etc/wpa_supplicant.conf
cp ${HERE}/init.d/* ${TARGET_DIR}/etc/init.d/
cp ${HERE}/sshd_config ${TARGET_DIR}/etc/ssh/
cp ${HERE}/{config.txt,cmdline.txt} ${BINARIES_DIR}/rpi-firmware/

for overlay in ${HERE}/overlays/*; do
  dtc -@ -I dts -O dtb -o ${BINARIES_DIR}/rpi-firmware/overlays/$(basename $overlay -overlay.dts).dtbo $overlay
done
