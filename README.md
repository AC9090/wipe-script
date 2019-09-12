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

modify code, then test it locally

sudo ./test_it.sh

Test it on the Red Network updating the SQL database

sudo ./run_it.sh

Update github 

git push origin master

make new image
--------------

Copy latest code to ramdisk

sudo ./move_it.sh

cd ramdisk_work

sudo ./buildrd.sh or ./buildiso

copy initrd to USB stick to transfer it to the Red Server or right click on iso and "Make bootable USB" 

On the Red Server login and make a backup of the existing image

cd /var/lib/tftpboot/stable

sudo cp -p initrd initrd_{date}

sudo cp /media/user/{USB name}/initrd .

Boot a test machine







