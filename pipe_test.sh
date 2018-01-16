#! /bin/bash

echo "SE Enabled 
ET 10min"

echo "SN HD10001"

for i in $(seq 1 100); do
	echo "LINE NUMBER $i"
	sleep 30
done

# echo "ER PANIC!!"

echo "\e[31mErase failed. Replace hard drive.\e[0m"

sleep 10