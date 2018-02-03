#!/bin/bash

MYSERVERIP="192.168.0.1"
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

# If cloning is chosen, choose source disk.

  will_clone=false
  if (whiptail --title "$brand" --yesno "Would you like to clone another drive onto selected?" 20 78); then
    will_clone=true

    # Strip disks that are selected for wipe from choices. 
    sources_available=()
    for drive in $drives_available; do
      not_sel=1
      for a_drive in $drives_selected; do
        if [[ "$drive" == "$a_drive" ]]; then
          not_sel=0
        fi
      done
      if [ $not_sel  -ne 0 ] && [ $drive !=  "on" ] && [[  -n  "`echo $drive | grep dev`" ]] ; then
        sources_available+="$drive "
      sources_available+="a "
      fi
    done

    source_drive=$(whiptail --title "$brand" --menu --noitem "\nPlease select a drive to clone.\n \
    " 22 78 12  $sources_available  3>&1 1>&2 2>&3)
    
    exitstatus=$?
    if [[ ( $exitstatus != 0 ) || ( -z $source_drive ) ]]; then
      echo
      echo "Shutting down (2)..."
      exit
    fi
  fi
  
  mount -t nfs -o proto=tcp,port=2049 $MYSERVERIP:/ $MYMOUNTPOINT

  has_parent=false
  if (whiptail --title "$brand" --yesno "Does this computer have an asset number associated with it?" 20 78); then
    has_parent=true

    parent=$(whiptail --inputbox "Please enter the asset number:" 8 78 1000 --title "$brand" 3>&1 1>&2 2>&3)
    computer_service_tag=$(whiptail --inputbox "Please enter the service tag (if it exits):" 8 78 --title "$brand" 3>&1 1>&2 2>&3)
    echo "Collecting device information..."
    computer_model=`lshw -short | grep system | awk '{for (i=2; i<NF; i++) printf $i " "; if (NF >= 4) print $NF; }'`
    computer_processor=`lshw -short | grep -m1 processor | awk '{for (i=3; i<NF; i++) printf $i " "; if (NF >= 4) print $NF; }'`

    ./sql-handler -c "$parent" "$computer_service_tag" "$computer_model" "$computer_processor"

    exitstatus=$?
    if [[ ( $exitstatus != 0 ) ]]; then
      echo
      echo "Could not update sql database. Shutting down..."
      exit
    fi
  fi

  # Wipe confirmation
  if (whiptail --title "$brand" --yesno "Are you sure you want to securely wipe the following drives:\n\n\
${drives_selected[@]} " 20 78); then
    echo
    echo "Confirmation given to begin wiping selected drives..."
  else
    echo
    echo "Shutting down..."
    exit
  fi


# Since implementation of later code seems to end up doubling up the "/dev/" in the paths it is removed
  drives_selected2=""
  for drive_path in $drives_selected; do
    for drive in $drives; do
      if [[ "$drive_path" -ef "/dev/$drive" ]]; then
        drives_selected2+="$drive "

      fi
    done
  done


  # Start the process handler
  if $will_clone  &&  $has_parent ; then
    echo $drives_selected2
    ./process-handler -c $source_drive -p $parent $drives_selected2
  elif  $will_clone  ; then
    ./process-handler -c $source_drive $drives_selected2
  elif  $has_parent ; then 
    ./process-handler -p $parent $drives_selected2
  else 
    ./process-handler $drives_selected2
  fi
  

  echo "All subprocesses are complete."
  
  sleep 10
fi



