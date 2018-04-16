#!/bin/bash

export NEWT_COLORS='
window=whit,black
border=white.black
textbox=white,black
button=white,red
'

# Initial loading message
tagline="wipe utility for SATA drives"
if [ -z "$1" ]; then
  brand="Parallel $tagline"
else
  brand="$1 parallel $tagline"
fi

dmesg -n 1

while true; do

	selection=$(whiptail --title "$brand" --menu "\nPlease select an option:\n " 22 78 12 \
		"Wipe" "Run the wipe script."\
		"Shell" "Show a bash shell." \
		"Disk Info" "Run 'hdparm -I' to get information on a disk." \
		"Shutdown" "Turn off the machine." \
		"Exit" "Exit the wipe script" \
		3>&1 1>&2 2>&3);
		#"Wipe Advanced" "Run the wipe sript with advanced options."

  	if [ "$selection" == "Wipe" ]; then
  		bash -c "./wipe-main.sh"

  	elif [ "$selection" == "Disk Info" ]; then
  		drives=`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | awk '{print $1}'`
		drives_count=(`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | grep -c sd`)
		drives_available=()
		for drive in $drives; do
			drives_available+="/dev/$drive \
			`lsblk -nio MODEL /dev/$drive | awk '{print $1}'`_`lsblk -nio SIZE,TYPE /dev/$drive | grep disk | awk '{print $1}'` "
		done

   		drive_selected=$(whiptail --title "$brand" --menu "\nPlease select a drive to get information from.\nUse up and down arrows to scroll and press 'q' to quit the info screen." \
   			22 78 12  $drives_available  3>&1 1>&2 2>&3)
    	hdparm -I $drive_selected | less

  	elif [ "$selection" == "Shutdown" ]; then
    	echo
    	echo "Shutting down..."
    	#shutdown now
    	exit

    elif [ "$selection" == "Exit" ]; then
    	echo
    	echo "Exiting..."
    	exit

    elif [ "$selection" == "Shell" ]; then
    	echo "Type exit to return to selection menu."
    	bash
  	fi


done