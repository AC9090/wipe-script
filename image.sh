#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi


drives=`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | awk '{print $1}'`
drives_count=(`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | grep -c sd`)
drives_available=()
for drive in $drives; do
 drives_available+="/dev/$drive \
 `lsblk -nio MODEL /dev/$drive | awk '{print $1}'`_`lsblk -nio SIZE,TYPE /dev/$drive | grep disk | awk '{print $1}'` "
done

whiptail --title "Image for mac" --infobox "To perform this imaging process please ensure that the disk has been wiped and a mac image usb stick is connected." 8 78
dest_drive=$(whiptail --title "$brand" --menu "\nPlease select a drive to image." \
	22 78 12  $drives_available  3>&1 1>&2 2>&3)

source_drive=$(whiptail --title "$brand" --menu "\nPlease select the source drive." \
	22 78 12  $drives_available  3>&1 1>&2 2>&3)

echo "o
n
p
1

+12000M
a
1
w
" | fdisk $dest_drive

disk_endB=`fdisk -l $dest_drive | grep "Disk $dest_drive:" | cut -d " " -f 5`

swap_startB=$(( $disk_endB - 5000000000 ))
echo $swap_startB
swap_startKB=$(( $swap_startB / 1000 ))
echo "n
p
3
+"$swap_startKB"KB

w
" | fdisk $dest_drive

echo "n
p
2


w
" | fdisk $dest_drive

partprobe $dest_drive

mkfs.ext4 "$dest_drive"1
mkfs.ext4 "$dest_drive"2
mkswap "$dest_drive"3

mount "$source_drive"1 /mnt/
echo "Imaging mint, please wait..."
dd if=/mnt/mint.img of="$dest_drive"1 bs=64K conv=sync,noerror
echo "Imaging rachel, please wait..."
dd if=/mnt/rachel.img of="$dest_drive"2 bs=64K conv=sync,noerror
umount /mnt/

mount "$dest_drive"1 /mnt/

mount --bind /dev /mnt/dev
mount --bind /sys /mnt/sys
mount --bind /proc /mnt/proc

chroot /mnt/ grub-install $dest_drive

uuid=`lsblk -no UUID /dev/sda3`
echo "UUID=$uuid none swap defaults 0 0" >> /mnt/etc/fstab
uuid=`lsblk -no UUID /dev/sda2`
chroot /mnt/ mkdir /home/user/Rachel
echo "UUID=$uuid /home/user/Rachel ext4 defaults 0 0" >> /mnt/etc/fstab
umount /mnt/dev
umount /mnt/sys
umount /mnt/proc
umount /mnt/

e2fsck -f "$dest_drive"1
e2fsck -f "$dest_drive"2
resize2fs "$dest_drive"1
resize2fs "$dest_drive"2

read -n 1 -s -r -p "Done, press any key to continue."