#! /bin/bash

echo "SE Enabled 
ET 100 minutes"

for i in $(seq 1 10); do
	echo "LINE NUMBER $i"
	sleep 1
done

echo "ER PANIC!!"