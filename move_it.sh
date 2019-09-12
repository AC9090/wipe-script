#!/bin/bash
# July 2019 philn
# August 2019 - GD added init
#
cp -p .bashrc init-wipe.sh process-handler sql-copy sql-handler unlock_drive.sh wipe-main.sh wipe-worker.sh ramdisk_work/chroot/root
cp -p init ramdisk_work/chroot
ls -la ramdisk_work/chroot/root
ls -la ramdisk_work/chroot | grep init

