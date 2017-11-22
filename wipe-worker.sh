#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

MYLOGFILENAME="/var/log/wipe.log"

drive=$1
source_drive=$2

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
# if [ $smart_check != 0 ]; then
#  echo "SMART status for device /dev/$drive: $Disk_Health"
# else
#  echo -e "\e[33mDevice /dev/$drive does not support SMART or it is disabled.\e[0m"
# fi

# If drive is healthy or if SMART is unavailable, check for security erase support and wipe using hdparm or nwipe
if [ $smart_check == 0 ] || [ $disk_health == PASSED ]; then
  if [ $security_erase != 0 ]; then
    # Run hdparm erase
    echo
    echo -e "\e[32mThis device supports security erase.\e[0m"

    # Check if drive is frozen and sleep machine if necessary
    if [ $disk_frozen == 0 ]; then
      echo
      echo "Device /dev/$drive is frozen. Sleeping machine to unfreeze..."
      #sleep 3s
      #rtcwake -u -s 10 -m mem >/dev/null
      #sleep 5s
    fi
    echo "Setting password..."
    echo
    hdparm --security-set-pass password /dev/$drive >/dev/null
    if [ $? -eq 0 ]; then
      echo -e "\e[32mPassword set\e[0m"
    else
      echo -e "\e[31mFailed to set password!\e[0m"
      echo
      read -p "Press any key to continue." -n1 -s
    fi
    echo
    MYTIMEVAR=`date +'%k:%M:%S'`
    if [ $Enhanced_Erase == 0 ]; then
      echo "Enhanced secure erase of $Disk_Model (/dev/$drive) started at $MYTIMEVAR." && echo "Wiping device using enhanced secure erase." >>  $MYLOGFILENAME && echo >> $MYLOGFILENAME
      if [[ $Erase_Estimate ]]; then
        echo "Estimated time for erase is $Erase_Estimate."
      else
        echo "Estimated time for erase is unknown. It may take one or more hours..."
      fi
      hdparm --security-erase-enhanced password /dev/$drive >/dev/null
    else
      echo "Secure erase of $Disk_Model (/dev/$drive) started at $MYTIMEVAR." && echo -e "This may take one or more hours..."  && echo "Wiping device using secure erase." >>  $MYLOGFILENAME && echo >> $MYLOGFILENAME
      if [[ $Erase_Estimate ]]; then
        echo "Estimated time for erase is $Erase_Estimate."
      else
        echo "Estimated time for erase is unknown. It may take one or more hours..."
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
    fi
  else
    #
    # Run nwipe
    #
    echo -e "\e[33mDevice /dev/$drive does not support security erase. Falling back to nwipe...\e[0m"
    echo
    sleep 3s
    nwipe --autonuke --method=dodshort --nowait --logfile=$MYLOGFILENAME /dev/$drive
    MYTIMEVAR=`date +'%a %d %b %Y %k:%M:%S'`
    #echo "Finished on: $MYTIMEVAR" >> $MYLOGFILENAME
    #echo "$NWIPEVERSION" >> $MYLOGFILENAME
  fi
fi

wait
#
# If SMART is supported and drive is unhealthy, print message to replace drive
#
if [ $smart_check != 0 ] && [ $disk_health != PASSED ]; then
  echo -e "\e[31mSMART check of /dev/$drive failed. Replace hard drive.\e[0m"
  echo
  read -p "Press any key to continue." -n1 -s
  exit
fi

MYTIMEVAR=`date +'%a %d %b %Y %k:%M:%S'`

# Cloning stage.
if [ -z $source_drive ]; then
  echo
  echo "Wipe stage complete. No source drive selected. Exiting..."
  exit
else
  echo "Cloning from source $source_drive to $drive"

  dd if=$source_drive of=/dev/$drive conv=noerror,sync bs=64K status=progress

  echo "Cloning from source to /dev/$drive complete."

fi
whiptail --title "$brand" --info "Operations Complete on /dev/$drive" 20 78)
echo "Operations complete. Exiting..."
sleep 6
exit