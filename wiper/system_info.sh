#/bin/bash
mysql -u wipe --password=wipepw -e "select disk_serial,disk_model,disk_size,parent,wiped from disk where parent like '%$1' order by wiped" wipedb

