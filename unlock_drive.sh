#!/bin/bash
# Unlock disk
# July 2019 philn
drives=`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | awk '{print $1}'`
drives_count=(`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | grep -c sd`)
drives_available=()
for drive in $drives; do
	drives_available+="/dev/$drive \
	`lsblk -nio MODEL /dev/$drive | awk '{print $1}'`_`lsblk -nio SIZE,TYPE /dev/$drive | grep disk | awk '{print $1}'` "
	done

drive_selected=$(whiptail --title "$brand" --menu "\nPlease select a drive to unlock\nUse up and down arrows to scroll and press 'q' to quit the info screen." \
   			22 78 12  $drives_available  3>&1 1>&2 2>&3)
echo "Drive selected $drive_selected"
hdparm --security-disable password $drive_selected
if [[ ( $exitstatus != 0 ) ]] ; then
      echo "Failed to unlock disk $drive_selected"
      echo
fi
read -p "Press [Enter] key to return to menu"
