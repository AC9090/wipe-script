#/bin/bash
mysql -u wipe --password=wipepw -e "select disk_serial,disk_model,parent,wiped from disk where disk_serial like '$1%' order by wiped" wipedb

