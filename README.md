# wipe-script

Run on 32 bit MINT

Install
-------

sudo apt install git

sudo apt install nwipe

sudo apt install smartmontools

sudo apt install libmysqlclient-dev

sudo apt install xorriso

Modify
---------------

cd /home/turing/Documents/wipe-script 

modify code, then test it some of it locally

Make new image
--------------

Copy latest code to ramdisk

sudo ./move_it.sh

cd ramdisk

sudo ./build.sh

This will copy initrd to /var/lib/tftboot/wipe and make an iso right click on it and "Make bootable USB" 

Boot a test machine







