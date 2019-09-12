#!/bin/bash
# 10/7/19 rationalize USESQL
MYSERVERIP="192.168.0.1"
VER=1.0
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


      parent=$(whiptail --inputbox "Please enter the asset number (double check your entry please):" 8 78 0000 --title "$brand" 3>&1 1>&2 2>&3)
      
      exitstatus=$?
      if [ $exitstatus != 0 ]; then
        exit
      fi
    fi
      
    service=`dmidecode | grep "System Information" -A 10 | grep "Serial Number" | awk -F ":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
    computer_service_tag=$(whiptail --inputbox "Please enter the service tag (if it exits):" 8 78 "$service" --title "$brand" 3>&1 1>&2 2>&3)
      
    #echo "Collecting device information..."
    computer_manufacturer=`dmidecode | grep -A3 '^System Information' | grep "Manufacturer" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
    computer_model=`dmidecode | grep -A3 '^System Information' | grep "Product Name" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
    computer_type=`dmidecode | grep -A3 '^Chassis Information' | grep "Type" | awk -F":" '{print $2}' | sed -e 's/^[ <t]*//;s/[ <t]*$//'`
    if [ $computer_type == Laptop ]; then  
        is_laptop=1
    else 
        is_laptop=0
    fi

    computer_processor=`lshw -short | grep -m1 processor | awk '{for (i=3; i<NF; i++) printf $i " "; if (NF >= 4) print $NF; }'`
    if [ $USESQL == true ]; then
        ./sql-handler -i -c asset_no="$parent" service_tag="$computer_service_tag" is_laptop="$is_laptop" make="$computer_manufacturer" model="$computer_model" processor="$computer_processor"
      
        exitstatus=$?
        if [ $exitstatus == 1 ]; then
          echo
          echo "Could not update sql database. Shutting down..."
          exit
        elif [ $exitstatus == 2 ]; then
          if (whiptail --title "$brand" --yesno "The asset number $parent is already recorded in the database. \
Please check you entered the correct asset number. Would you like to continue? " 20 78); then
            ./sql-handler -u -c asset_no="$parent" service_tag="$computer_service_tag" is_laptop="$is_laptop" model="$computer_model"  make="$computer_manufacturer" processor="$computer_processor"
          else 
            echo "Shutting down..."
            exit
          fi
        fi
    else
        echo asset_no="$parent" service_tag="$computer_service_tag" is_laptop="$is_laptop" make="$computer_manufacturer" model="$computer_model" processor="$computer_processor"
    fi

    has_parent=true

   nwipe

   echo "Finished."
   dmesg -n warn
   sleep 1



