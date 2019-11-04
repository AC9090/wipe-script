#!/bin/bash
# 
# 25/6/19 Remove cloning
# 10/7/19 Get is_laptop from bios
# 20/8/19 GD fixed a few flow controll issues. 
#            Added sleeps to allow echos to be read.
#            Added code to optionally allow drive to be wiped even if SQL fails.
#
# 26/8/19 GD Added code to select between hdparm wipe and using two-pass dd. Note the selection applies to
#	     all selected drives
#
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
  whiptail --title "$brand" --msgbox "No SATA drives detected. Select Ok to exit." 8 78
  echo
  echo "Exiting........."
  sleep 2
  exit
else

  # Allow selection of drives to wipe
  drives_selected=$(whiptail --title "$brand" --checklist --separate-output "\n$drives_count drives are connected. \
The selected drives will be wiped in parallel." 22 78 12 $drives_available 3>&1 1>&2 2>&3)

  exitstatus=$?
  if [[ ( $exitstatus != 0 ) || ( -z $drives_selected ) ]]; then
    echo
    echo "Exiting........."
    sleep 2
    exit
  fi

  has_parent=false
  if (whiptail --title "$brand" --yesno "Does this computer have an asset number associated with it?" 20 78); then
      has_parent=true
      parent=$(whiptail --inputbox "Please enter the asset number (double check your entry please):" 8 78 0000 --title "$brand" 3>&1 1>&2 2>&3)
      
      exitstatus=$?
      if [ $exitstatus != 0 ]; then
        exit
      fi
      
      service=`dmidecode | grep "System Information" -A 10 | grep "Serial Number" | awk -F ":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
      computer_service_tag=$(whiptail --inputbox "Please enter the service tag (if it exits):" 8 78 "$service" --title "$brand" 3>&1 1>&2 2>&3)
      #if (whiptail --title "$brand" --yesno "Is this computer a laptop?" 20 78 ); then  
      #  is_laptop=1
      #else 
      #  is_laptop=0
      #fi

      echo "Collecting device information..."
      computer_manufacturer=`dmidecode | grep -A3 '^System Information' | grep "Manufacturer" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
      computer_model=`dmidecode | grep -A3 '^System Information' | grep "Product Name" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
      system_type=`dmidecode | grep -A3 '^Chassis Information' | grep "Type" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
      echo "System type $system_type"
      is_laptop=0
      if [ $system_type == Laptop ]; then
        is_laptop=1
      fi
      computer_processor=`lshw -short | grep -m1 processor | awk '{for (i=3; i<NF; i++) printf $i " "; if (NF >= 4) print $NF; }'`

# Removed USESQL usage GD 15/8/19

      ./sql-handler -i -c asset_no="$parent" service_tag="$computer_service_tag" is_laptop="$is_laptop" make="$computer_manufacturer" model="$computer_model" processor="$computer_processor"

       exitstatus=$?

       if [ $exitstatus == 1 ]; then

          if (whiptail --title "$brand" --yesno "unable to update SQL. Do you want to continue?" 20 78 ); then  
              echo "Continuing"
	      sleep 2
	  else
              exit
          fi

       elif [ $exitstatus == 2 ]; then
          if (whiptail --title "$brand" --yesno "The asset number $parent is already recorded in the database. \
		Please check you entered the correct asset number. Would you like to continue? " 20 78); then
            ./sql-handler -u -c asset_no="$parent" service_tag="$computer_service_tag" is_laptop="$is_laptop" model="$computer_model"  make="$computer_manufacturer" processor="$computer_processor"
          else 
            echo "Exiting........."
	    sleep 2
            exit
          fi
       else
          ./sql-handler -u -c asset_no="$parent" service_tag="$computer_service_tag" is_laptop="$is_laptop" model="$computer_model"  make="$computer_manufacturer" processor="$computer_processor"
       fi
  fi
  # Wipe confirmation
  if (whiptail --title "$brand" --yesno "Are you sure you want to securely wipe the following drives:\n\n\
${drives_selected[@]} " 20 78); then
    echo
    echo "Confirmation given to begin wiping selected drives..."
    sleep 2
  else
    echo
    echo "Exiting    ..."
    sleep 2
    exit
  fi

  #if (whiptail --title "$brand" --yesno "Using secure erase - OK? Select No if you need to use dd due to \
  #a problem with this computer when sleeping. " 20 78); then
    export Use_ddwipe="No"
# else 
#    export Use_ddwipe="Yes"
# fi

# Since implementation of later code seems to end up doubling up the "/dev/" in the paths it is removed
  drives_selected2=""
  for drive_path in $drives_selected; do
    for drive in $drives; do
      if [[ "$drive_path" -ef "/dev/$drive" ]]; then
        drives_selected2+="$drive "

      fi
    done
  done

pwd
echo "Drives $drives_selected2"
  # Start the process handler
  if  $has_parent ; then 
    echo "Has parent"
    ./process-handler -p $parent $drives_selected2
  else 
    echo "Has no parent"
    ./process-handler $drives_selected2
  fi
  

  echo "All subprocesses are complete."
  dmesg -n warn
  sleep 1
fi



