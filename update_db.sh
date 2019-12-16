#!/bin/bash
# 3/11/19 philn
# V1.1 philn. Add wipe to sqlconnect disk. This automagically adds timestamp
MYSERVERIP="192.168.0.1"
VER=1.1
NWIPELOG=nwipe.log
MYLOGFILENAME="wipe.log"
rm $MYLOGFILENAME
touch $MYLOGFILENAME
echo "nwipe script version $VER"
# Exit if not run as root (sudo)
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

#Supress kernel messages
dmesg -n 1

# Get some useful system variables
current_time=`date +'%a %d %b %Y %k:%M:%S'`
#read -p "Please enter barcode, or press CTRL C to exit: " -r BARCODE
#BARCODE=`echo "$BARCODE" | tr -cd [:alnum:] | tr '[:lower:]' '[:upper:]'`
export ver_nwipe=`nwipe --version`
export ver_hdparm=`hdparm -V`
  
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
      echo "asset_no=$parent service_tag=$computer_service_tag is_laptop=$is_laptop model=$computer_model  make=$computer_manufacturer processor=$computer_processor" >> $MYLOGFILENAME
      ./sql-handler -i -c asset_no="$parent" service_tag="$computer_service_tag" is_laptop="$is_laptop" make="$computer_manufacturer" model="$computer_model" processor="$computer_processor"
      exitstatus=$?
      echo "SQL status $exitstatus" >> $MYLOGFILENAME
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

wiped_drives=`dir /dev/sd* | tr -d 1`

for drive in $wiped_drives; do
  echo "Wiped $drive" >> $MYLOGFILENAME
  disk_model=`hdparm -I $drive | grep "Model Number" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
  disk_serial=`hdparm -I $drive | grep "Serial Number" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
  if [ -z $disk_serial ] ; then
    echo "Could not retrieve disk serial."
    echo "This could indicate the disk has failed"
    echo "Disk cannot be logged without a valid serial number"
    echo "Quitting"
    sleep 5
    exit
  fi

  disk_size=`hdparm -I $drive | grep 1000: | grep -oP '(?<=\()[^\)]+'`
  sata_count=`hdparm -I $drive | grep "Transport" | awk -F":" '{print $2}' | grep -c SATA`
  if [ $sata_count != 0 ]; then
    transport="SATA"
  else 
    transport="IDE"
  fi
  drive_letter=`echo $drive | sed -e 's?/dev/??'`
  rotational=`cat /sys/block/$drive_letter/queue/rotational`
  disk_health=`smartctl -H /dev/$drive_letter | grep -i "test result" | tail -c15 |awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
  form_factor=`hdparm -I $drive | grep "Form Factor" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
  rpm=`hdparm -I $drive | grep "Nominal Media Rotation Rate" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
  echo "disk_model=$disk_model disk_serial=$disk_serial disk_size=$disk_size disk_health=$disk_health rotational=$rotational" >> $MYLOGFILENAME
  ./sql-handler -u -d disk_model="$disk_model" disk_serial="$disk_serial" disk_size="$disk_size" security_erase="" enhanced_erase="" \
  health="$disk_health" source_drive="$source_drive_serial" parent="$parent" firmware="$firmware" rotational="$rotational" transport="$transport" \
  form_factor="$form_factor" rpm="$rpm" wiped
done
echo "Finished."
dmesg -n warn
sleep 1




