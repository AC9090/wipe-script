#!/bin/bash
# 10/7/19 rationalize USESQL
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


# Inv0ert enhanced erase since it is true if it's not enabled.
if [ $enhanced_erase != 0 ]; then
  enhanced_erase=0
else 
  enhanced_erase=1
fi
erase_estimate=`hdparm -I /dev/$drive | grep -i "for security erase" | awk '{print $1}'`
security_erase=`hdparm -I /dev/$drive | grep -c "Security Mode"`
smart_check=`hdparm -I /dev/$drive | grep -i "SMART feature set" | grep -c "*"`

firmware=`hdparm -I /dev/$drive | grep "Firmware Revision" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`

sata_count=`hdparm -I /dev/$drive | grep "Transport" | awk -F":" '{print $2}' | grep -c SATA`

if [ $sata_count != 0 ]; then
  transport="SATA"
else 
  transport="IDE"
fi

rotational=`cat /sys/block/$drive/queue/rotational`


form_factor=`hdparm -I /dev/$drive | grep "Form Factor" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
rpm=`hdparm -I /dev/$drive | grep "Nominal Media Rotation Rate" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`

if ! [[ $rpm =~ ^[0-9]+$ ]] ; then
  rpm=0
fi

if [ $USESQL == true ]; then
  ./sql-handler -u -d disk_model="$disk_model" disk_serial="$disk_serial" disk_size="$disk_size" security_erase="$security_erase" enhanced_erase="$enhanced_erase" \
health="$disk_health" source_drive="$source_drive_serial" parent="$parent" firmware="$firmware" rotational="$rotational" transport="$transport" form_factor="$form_factor" rpm="$rpm"
else
  echo "disk_model=$disk_model disk_serial=$disk_serial disk_size=$disk_size security_erase=$security_erase enhanced_erase=$enhanced_erase \
health=$disk_health source_drive=$source_drive_serial parent=$parent firmware=$firmware rotational=$rotational transport=$transport form_factor=$form_factor rpm=$rpm"
fi

if [ -z $disk_serial ] ; then
  echo "Could not retrieve disk serial."
  echo "This could indicate the disk has failed"
  echo "ER Disk serial unknown."
  exit
fi

echo "SN $disk_serial"

# Check if drive is locked and unlock if necessary
if [ $security_erase != 0 ] && [ $disk_lock == 0 ]; then
  echo "Unlocking device /dev/$drive..."
  hdparm --security-disable password /dev/$drive >/dev/null
  echo
fi

# Check SMART status
if [ -z $disk_health ] ; then
  echo "Could not retrieve disk health."
  echo "It may work if you repeat the wipe."
  echo "ER Disk health unknown."
  exit
fi

if [ $smart_check != 0 ]; then
  echo "SMART status for device /dev/$drive: $disk_health"
else
  echo -e "Device /dev/$drive does not support SMART or it is disabled."
fi



# If drive is healthy or if SMART is unavailable, check for security erase support and wipe using hdparm or nwipe
if [ $smart_check == 0 ] || [ $disk_health == PASSED ]; then
  if [ $security_erase != 0 ]; then
    # Run hdparm erase
    echo  
    echo -e "This device supports Secure Erase."

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
      echo "Password set."
    else
      echo "ER Failed to set password."
      echo
      
    fi
    echo
    MYTIMEVAR=`date +'%k:%M:%S'`
    if [ $enhanced_erase != 0 ]; then
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
      echo -e "Disk erased successfully." && echo "Blanked device successfully." >> $MYLOGFILENAME && echo >> $MYLOGFILENAME
      echo
    else
      echo  "ER hdparm returned error: $?"
      echo -e "Erase failed. Replace hard drive." && echo "Wipe of device failed." >> $MYLOGFILENAME && echo >> $MYLOGFILENAME
      echo
      exit
    fi

    # Ensure drive is unlocked.
    hdparm --security-set-pass password /dev/$drive >/dev/null
    hdparm --security-disable password /dev/$drive
    if [[ ( $exitstatus != 0 ) ]] ; then
      echo "Failed to unlock disk."
      echo
    fi
    
  else
    #
    # Run nwipe
    #
    echo "SE Disabled"
    echo -e "Device /dev/$drive does not support security erase. Falling back to nwipe..."
    echo "ET Unknown"
    echo "Note that an 80 GB drive can take more than 2 hours to wipe with nwipe. \n  To cancel before finish: \n wait until all other disks are done \n then press CTRL^c "
    sleep 3s
    nwipe --autonuke --method=dodshort --nowait --nogui --logfile=$MYLOGFILENAME /dev/$drive
    MYTIMEVAR=`date +'%a %d %b %Y %k:%M:%S'`
    #echo "Finished on: $MYTIMEVAR" >> $MYLOGFILENAME
    #echo "$NWIPEVERSION" >> $MYLOGFILENAME

    if [ $? -eq 0 ]; then
      echo
      echo "Nwipe completed successfully."
      echo
    else
      echo
      echo "ER Nwipe failed with error: $?."
      echo
      exit
    fi
  fi
fi

#
# If SMART is supported and drive is unhealthy, print message to replace drive
#
if [ $smart_check != 0 ] && [ $disk_health != PASSED ]; then
  echo -e "SMART check of /dev/$drive failed. Replace hard drive."
  echo  "ER SMART check failed."
  #read -p "Press any key to continue." -n1 -s
  exit
fi

MYTIMEVAR=`date +'%a %d %b %Y %k:%M:%S'`

# Cloning stage.
if [ "$source_drive" == "NONE" ]; then
  echo
  echo "Wipe stage complete. No source drive selected."
  echo  
  source_drive_serial="NONE"

else
  echo "Cloning from source $source_drive to $drive"

  dd if=$source_drive of=/dev/$drive conv=noerror,sync bs=64K #status=progress

  echo "Cloning from source to /dev/$drive complete."

  source_drive_serial=`hdparm -I /dev/$source_drive | grep "Serial Number" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
fi

if [ $USESQL == true ]; then
  ./sql-handler -u -d disk_serial="$disk_serial" wiped
exitstatus=$?
  if [[ ( $exitstatus != 0 ) ]]; then
    echo
    echo "Could not update sql database. Shutting down..."
    exit
  else 
    echo "Database updated."
  fi
else
  echo "disk_serial=$disk_serial wiped"
fi

echo "Operations complete. Exiting..."
sleep 6
exit
