#!/bin/bash
# 11/7/19 Add nwipe as menu option
# 14/7/19 Get shell menu option working
# 29/10/19 Version 2.1 Add clear in shell

version=V2.1
export NEWT_COLORS='
window=white,black
border=white.black
textbox=white,black
button=white,red
'

# Initial loading message
tagline="Turing Trust Disk Wipe Utility $version"
if [ -z "$1" ]; then
  brand="$tagline"
else
  brand="$1 parallel $tagline $version"
fi

dmesg -n 1


while true; do

	selection=$(whiptail --title "$brand" --menu "\nPlease select an option:\n " 22 78 12 \
		"Wipe" "Run the Secure Erase script."\
        	"Nwipe" "Run Disk Wipe script."\
		"Shell" "Show a bash shell." \
        	"Unlock" "Unlock a disk." \
		"Disk Info" "Run 'hdparm -I' to get information on a disk." \
		"Shutdown" "Turn off the machine." \
		"About" "Info about the wipe script" \
		3>&1 1>&2 2>&3);
		#"Wipe Advanced" "Run the wipe sript with advanced options."
  	if [ "$selection" == "Wipe" ]; then
  		bash -c "./wipe-main.sh"

    	elif [ "$selection" == "Nwipe" ]; then
       		 bash -c "./nwipe-script.sh"

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

    	elif [ "$selection" == "About" ]; then
            
    		whiptail --title "Info" --msgbox "`cat README.txt`" 20 78 --scrolltext

    	elif [ "$selection" == "Shell" ]; then
        	clear
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
