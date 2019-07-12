# wipe-script

Run on 32 bit MINT

Install
-------

sudo apt install git

sudo apt install nwipe

sudo apt-get install smartmontools

sudo apt install libmysqlclient-dev

Modify
---------------

cd /home/turing/Documents/wipe-script 

modify code, then test it locally

sudo ./test_it.sh

Test it on the Red Network updating the SQL database

sudo ./run_it.sh

Update github 

git push origin master

Install it into ramdisk
-----------------------

Copy latest code to ramdisk

sudo ./move_it.sh



