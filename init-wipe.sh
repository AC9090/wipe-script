#!/bin/bash
# 11/7/19 Add nwipe as menu option
# 14/7/19 Get shell menu option working
# 27/8/19 GD Remove nwipe option since it has been incorporated
#	into the main secure wipe script as an option and
#       rerplaced by dd

export NEWT_COLORS='
window=white,black
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

# Nwipe incorporated into
# main wipe script and replaced by dd
#		"Nwipe" "Run Disk Wipe script."

	selection=$(whiptail --title "$brand" --menu "\nPlease select an option:\n " 22 78 12 \
		"Wipe" "Run the Secure Erase script."\
		"Shell" "Show a bash shell." \
        	"Unlock" "Unlock a disk." \
		"Disk Info" "Run 'hdparm -I' to get information on a disk." \
		"Shutdown" "Turn off the machine." \
		"Exit" "Exit the wipe script" \
		3>&1 1>&2 2>&3);
  	if [ "$selection" == "Wipe" ]; then
  		bash -c "./wipe-main.sh"

#    	elif [ "$selection" == "Nwipe" ]; then
#      		 bash -c "./nwipe-script.sh"

    	elif [ "$selection" == "Unlock" ]; then
        	bash -c "./unlock_drive.sh"

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
		if [ "x$drive_selected" == "x" ]; then
		    echo "No drive selected"
		    sleep 2
		else
    		    hdparm -I $drive_selected | less
		fi

  	elif [ "$selection" == "Shutdown" ]; then
    	    echo
    	    echo "Shutting down..."
	    sleep 2
	    shutdown -h 0
    	    exit

    	elif [ "$selection" == "Exit" ]; then
    		echo
       		echo
        	echo
    	 	echo "Type exit to return to wipe utility menu."
    		exit

    	elif [ "$selection" == "Shell" ]; then
        	echo
        	echo
        	echo
    		echo "Type exit to return to selection menu."
		sleep 2
    		bash --norc --noprofile
    	else
        	echo "Selection " $selection
		sleep 2
  	fi


done
