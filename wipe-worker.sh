#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

MYLOGFILENAME="/var/log/wipe.log"

drive=$1
source_drive=$2
parent=$3

disk_frozen=`hdparm -I /dev/$drive | grep frozen | grep -c not`
disk_health=`smartctl -H /dev/$drive | grep -i "test result" | tail -c15 |awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
disk_lock=`hdparm -I /dev/$drive | grep locked | grep -c not`
disk_model=`hdparm -I /dev/$drive | grep "Model Number" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
disk_serial=`hdparm -I /dev/$drive | grep "Serial Number" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
disk_size=`hdparm -I /dev/$drive | grep 1000: | grep -oP '(?<=\()[^\)]+'`
enhanced_erase=`hdparm -I /dev/$drive | grep -i enhanced | grep -c not`
erase_estimate=`hdparm -I /dev/$drive | grep -i "for security erase" | awk '{print $1}'`
security_erase=`hdparm -I /dev/$drive | grep -c "Security Mode"`
smart_check=`hdparm -I /dev/$drive | grep -i "SMART feature set" | grep -c "*"`

# Check if drive is locked and unlock if necessary
if [ $security_erase != 0 ] && [ $disk_lock == 0 ]; then
  echo "Unlocking device /dev/$drive..."
  hdparm --security-disable password /dev/$drive >/dev/null
  echo
fi

# Check SMART status
if [ $smart_check != 0 ]; then
  echo "SMART status for device /dev/$drive: $Disk_Health"
else
  echo -e "\e[33mDevice /dev/$drive does not support SMART or it is disabled.\e[0m"
fi

# If drive is healthy or if SMART is unavailable, check for security erase support and wipe using hdparm or nwipe
echo "SN $disk_serial"
if [ $smart_check == 0 ] || [ $disk_health == PASSED ]; then
  if [ $security_erase != 0 ]; then
    # Run hdparm erase
    echo  
    echo -e "\e[32mThis device supports security erase.\e[0m"

    # Check if drive is frozen and sleep machine if necessary
    if [ $disk_frozen == 0 ]; then
      echo
      echo "Device /dev/$drive is frozen. Sleeping machine to unfreeze..."
      sleep 3s
      rtcwake -u -s 10 -m mem >/dev/null
      sleep 5s
    fi
    echo "Setting password..."
    echo
    hdparm --security-set-pass password /dev/$drive >/dev/null
    if [ $? -eq 0 ]; then
      echo -e "\e[32mPassword set\e[0m"
    else
      echo -e "\e[31mFailed to set password!\e[0m"
      echo
      
    fi
    echo
    MYTIMEVAR=`date +'%k:%M:%S'`
    if [ $enhanced_erase == 0 ]; then
      echo "SE Enhanced"
      echo "Enhanced secure erase of $Disk_Model (/dev/$drive) started at $MYTIMEVAR." && echo "Wiping device using enhanced secure erase." >>  $MYLOGFILENAME && echo >> $MYLOGFILENAME
      if [[ $erase_estimate ]]; then
        echo "Estimated time for erase is $erase_estimate."
        echo "ET $erase_estimate"

      else
        echo "Estimated time for erase is unknown. It may take one or more hours..."
        echo "ET Unknown"
      fi
      hdparm --security-erase-enhanced password /dev/$drive >/dev/null
    else
      echo "SE Enabled"
      echo "Secure erase of $Disk_Model (/dev/$drive) started at $MYTIMEVAR." && echo -e "This may take one or more hours..."  && echo "Wiping device using secure erase." >>  $MYLOGFILENAME && echo >> $MYLOGFILENAME
      if [[ $erase_estimate ]]; then
        echo "Estimated time for erase is $erase_estimate."
        echo "ET $erase_estimate"
      else
        echo "Estimated time for erase is unknown. It may take one or more hours..."
        echo "ET Unknown"
      fi
      hdparm --security-erase password /dev/$drive >/dev/null 
    fi
    if [ $? -eq 0 ]; then
      echo
      echo -e "\e[32mDisk erased successfully.\e[0m" && echo "Blanked device successfully." >> $MYLOGFILENAME && echo >> $MYLOGFILENAME
      echo
    else
      echo
      echo -e "\e[31mErase failed. Replace hard drive.\e[0m" && echo "Wipe of device failed." >> $MYLOGFILENAME && echo >> $MYLOGFILENAME
      echo
      exit
    fi
  else
    #
    # Run nwipe
    #
    echo "Disabled"
    echo -e "\e[33mDevice /dev/$drive does not support security erase. Falling back to nwipe...\e[0m"
    echo "ET Unknown"
    echo
    sleep 3s
    nwipe --autonuke --method=dodshort --nowait --logfile=$MYLOGFILENAME /dev/$drive
    MYTIMEVAR=`date +'%a %d %b %Y %k:%M:%S'`
    #echo "Finished on: $MYTIMEVAR" >> $MYLOGFILENAME
    #echo "$NWIPEVERSION" >> $MYLOGFILENAME

    if [ $? -eq 0 ]; then
      echo
      echo "Nwipe completed successfully."
      echo
    else
      echo
      echo "ER Nwipe failed."
      echo
      exit
    fi
  fi
fi

#
# If SMART is supported and drive is unhealthy, print message to replace drive
#
if [ $smart_check != 0 ] && [ $disk_health != PASSED ]; then
  echo -e "\e[31mSMART check of /dev/$drive failed. Replace hard drive.\e[0m"
  echo  "ER SMART check failed."
  read -p "Press any key to continue." -n1 -s
  exit
fi

MYTIMEVAR=`date +'%a %d %b %Y %k:%M:%S'`

# Cloning stage.
if [ "$source_drive" == "NONE" ]; then
  echo
  echo "Wipe stage complete. No source drive selected."
  echo  
else
  echo "Cloning from source $source_drive to $drive"

  dd if=$source_drive of=/dev/$drive conv=noerror,sync bs=64K status=progress

  echo "Cloning from source to /dev/$drive complete."

fi

./sql-handler -d "$disk_model" "$disk_serial" "$disk_size" "$security_erase" "$enhanced_erase" "$source_drive" "$parent"

exitstatus=$?
if [[ ( $exitstatus != 0 ) ]]; then
  echo
  echo "Could not update sql database. Shutting down..."
  exit
else 
  echo "Database updated."
fi

echo "Operations complete. Exiting..."
sleep 6
exit
