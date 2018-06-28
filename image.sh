#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi


# drives=`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | awk '{print $1}'`
# drives_count=(`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | grep -c sd`)
# drives_available=()
# for drive in $drives; do
#  drives_available+="/dev/$drive \
#  `lsblk -nio MODEL /dev/$drive | awk '{print $1}'`_`lsblk -nio SIZE,TYPE /dev/$drive | grep disk | awk '{print $1}'` "
# done

# drive_selected=$(whiptail --title "$brand" --menu "\nPlease select a drive to image." \
# 	22 78 12  $drives_available  3>&1 1>&2 2>&3)
whiptail --title "Image for mac" --infobox "To perform this imaging process please ensure that the disk has been wiped and a mac image usb stick is connected." 8 78
sleep 5

drive_selected="/dev/sda"

echo "o
n
p
1

+12000M
a
1
w
" | fdisk $drive_selected

disk_endB=`fdisk -l $drive_selected | grep "Disk $drive_selected:" | cut -d " " -f 5`

swap_startB=$(( $disk_endB - 5000000000 ))
echo $swap_startB
swap_startKB=$(( $swap_startB / 1000 ))
echo "n
p
3
+"$swap_startKB"KB

w
" | fdisk $drive_selected

echo "n
p
2


w
" | fdisk $drive_selected

partprobe /dev/sda

mkfs.ext4 "$drive_selected"1
mkfs.ext4 "$drive_selected"2
mkswap "$drive_selected"3

mount /dev/sdb1 /mnt/
echo "Imaging mint, please wait..."
dd if=/mnt/mint.img of="$drive_selected"1 bs=64K conv=sync,noerror
echo "Imaging rachel, please wait..."
dd if=/mnt/rachel.img of="$drive_selected"2 bs=64K conv=sync,noerror
umount /mnt/

mount "$drive_selected"1 /mnt/

mount --bind /dev /mnt/dev
mount --bind /sys /mnt/sys
mount --bind /proc /mnt/proc

chroot /mnt/ grub-install $drive_selected

uuid=`lsblk -no UUID /dev/sda3`
echo "UUID=$uuid none swap defaults 0 0" >> /mnt/etc/fstab
uuid=`lsblk -no UUID /dev/sda2`
chroot /mnt/ mkdir /home/user/Rachel
echo "UUID=$uuid /home/user/Rachel ext4 defaults 0 0" >> /mnt/etc/fstab
umount /mnt/dev
umount /mnt/sys
umount /mnt/proc
umount /mnt/

e2fsck -f "$drive_selected"1
e2fsck -f "$drive_selected"2
resize2fs "$drive_selected"1
resize2fs "$drive_selected"2

read -n 1 -s -r -p "Done, press any key to continue."