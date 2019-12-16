#!/bin/bash
# July 2019 philn
# August 2019 - GD added init
# 29/10/19 PN added nwipe-script.sh
# 3/11/19 PN added about 
#
ramdisk="ramdisk"
cp -p .bashrc init-wipe.sh nwipe-script.sh process-handler sql-copy sql-handler unlock_drive.sh update_db.sh wipe-main.sh wipe-worker.sh $ramdisk/chroot/root
cp -p init $ramdisk/chroot
cp -p $ramdisk/README.txt $ramdisk/chroot/root
echo ls -la $ramdisk/chroot/root
ls -la $ramdisk/chroot/root
ls -la $ramdisk/chroot | grep init

