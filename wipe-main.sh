#!/bin/bash

# Exit if not run as root (sudo)
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# Initial loading message
tagline="wipe utility for SATA drives"
if [ -z "$1" ]; then
  brand="Parallel $tagline"
else
  brand="$1 parallel $tagline"
fi
bold=$(tput bold)
normal=$(tput sgr0)
echo
echo "Loading ${bold}$brand${normal}..."

# Get some useful system variables
current_time=`date +'%a %d %b %Y %k:%M:%S'`
#read -p "Please enter barcode, or press CTRL C to exit: " -r BARCODE
#BARCODE=`echo "$BARCODE" | tr -cd [:alnum:] | tr '[:lower:]' '[:upper:]'`
export ver_nwipe=`nwipe --version`
export ver_hdparm=`hdparm -V`

drives=`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | awk '{print $1}'`
drives_count=(`lsblk -nio KNAME,TYPE,SIZE,MODEL | grep disk | grep -c sd`)
drives_available=()
for drive in $drives; do
  drives_available+="/dev/$drive \
`lsblk -nio MODEL /dev/$drive | awk '{print $1}'`_`lsblk -nio SIZE,TYPE /dev/$drive | grep disk | awk '{print $1}'`\
 on "
done

# echo $drives_available
# exit

#drives=("sda" "Maxtor 80gb" "on" "sdb" "IBM 120Gb" "on" "sdc" "WD 300Gb" "on")
#drives_count=3

if [ $drives_count == 0 ]; then
  # No drives detected
  whiptail --title "$brand" --msgbox "No SATA drives detected. Select Ok to shut down." 8 78
  echo
  echo "Shutting down..."
  #shutdown now
  exit
else
#   echo whiptail --title \"$brand\" --checklist --separate-output \"\n$drives_count drives are connected. \
# The selected drives will be wiped in parallel.\" 22 78 12 $drives_available 3>&1 1>&2 2>&3

  # Allow selection of drives to wipe
  drives_selected=$(whiptail --title "$brand" --checklist --separate-output "\n$drives_count drives are connected. \
The selected drives will be wiped in parallel." 22 78 12 $drives_available 3>&1 1>&2 2>&3)

  exitstatus=$?
  if [[ ( $exitstatus != 0 ) || ( -z $drives_selected ) ]]; then
    echo
    echo "Shutting down..."
    #shutdown now
    exit
  fi

  sources_available=""
  for drive in $drives_available; do
  	not_sel=1
  	for a_drive in $drives_selected; do
  		if [[ "$drive" == "$a_drive" ]]; then
  			not_sel=0
  		fi
  	done
  	if [ $not_sel  -ne 0 ]; then
  		sources_available+="$drive -"
  	fi
  done
  
  #sources_available=("asdfkljs0dev/sda" "" "deasfa/sdb" "" "adfasev/sdc" "")
  
  #echo ${drives_available[0]}

  #source_drive=$(whiptail --title "$brand" --menu --noitem "\nPlease select a drive to clone.\n \
  #Skip to wipe only" 22 78 12 "${drives_available[@]}"  2>&1 1>&2)
  
  #exitstatus=$?
  #if [[ ( $exitstatus != 0 ) || ( -z $source_drive ) || ( $source_drive > 1 ) ]]; then
  #  echo
  #  echo "Shutting down (2)..."
    #shutdown now
  #  exit
  #fi
  source_drive="/dev/sdb"

  # Wipe confirmation
  if (whiptail --title "$brand" --yesno "Are you sure you want to securely wipe the following drives:\n\n\
${drives_selected[@]}" 20 78); then
    echo
    echo "Confirmation given to begin wiping selected drives..."
  else
    echo
    echo "Shutting down..."
    #shutdown now
    exit
  fi
# Since implementation of later code seems to end up doubling up the "/dev/" in the paths it is removed
	for drive_path in $drives_selected; do
		for drive in $drives; do
			if [[ "$drive_path" -ef "/dev/$drive" ]]; then
				echo "Starting worker for drive ${drive}..."
				sudo bash ./wipe-worker.sh $drive $source_drive &
			fi
		done
	done
fi