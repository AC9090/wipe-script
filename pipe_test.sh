#! /bin/bash

echo "SE Enabled 
ET 8min"

echo "SN HD10001"

for i in $(seq 1 120); do
	echo "LINE NUMBER $i"
	sleep 4
done

# echo "ER PANIC!!"

echo "\e[31mErase failed. Replace hard drive.\e[0m"

sleep 10

exit 0