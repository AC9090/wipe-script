#!/bin/bash
echo "Restart Networking"
echo " Q to continue"
sudo service isc-dhcp-server restart
sudo service tftpd-hpa restart
sudo service smbd restart
sudo service nmbd restart
sudo service isc-dhcp-server status
sudo service tftpd-hpa status
sudo service smbd status
sudo service nmbd restart
