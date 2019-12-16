#!/bin/bash
# GD Sept 2019
# This script moves other scripts into the working chroot directory, then
# builds that chroot into an initrd and then runs the ISO burn utility
# to build a bootable USB drive
#
cd ~/Documents/wipe-script
sudo ./move_it.sh
cd ramdisk_work
sudo ./build.sh
sudo mintstick -m iso
cd ~/Documents/wipe-script
echo
echo "Build Complete"
echo
exit
