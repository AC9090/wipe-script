#! /bin/bash

echo "SE Enabled 
ET 100 minutes"

echo "SN HD10001"

for i in $(seq 1 10); do
	echo "LINE NUMBER $i"
	sleep 1
done

# echo "ER PANIC!!"

echo "\e[31mErase failed. Replace hard drive.\e[0m"

sleep 10