#/bin/bash
echo "database dump of the wipedb, copy to RED_USB"
date
mysql -u wipe --password=wipepw  -e "select disk_model,disk_serial,disk_size,security_erase,enhanced_erase,health,rotational,source_drive,parent,wiped,disk.synced,firmware,form_factor,rpm,disk.sync_time,asset_no,service_tag,is_laptop,make,model,processor from disk left join computer on disk.parent=computer.asset_no order by wiped"  wipedb > disk_dump.csv
cp disk_dump.csv /media/user/RED_USB
umount /media/user/RED_USB
echo "finished. RED_USB can be removed"
