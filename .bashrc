# ~/.bashrc: executed by bash(1) for non-login shells.

# Note: PS1 and umask are already set in /etc/profile. You should not
# need this unless you want different defaults for root.
# PS1='${debian_chroot:+($debian_chroot)}\h:\w\$ '
# umask 022

# You may uncomment the following lines if you want `ls' to be colorized:
# export LS_OPTIONS='--color=auto'
# eval "`dircolors`"
# alias ls='ls $LS_OPTIONS'
# alias ll='ls $LS_OPTIONS -l'
# alias l='ls $LS_OPTIONS -lA'
#
# Some more alias to avoid making mistakes:
# alias rm='rm -i'
# alias cp='cp -i'
# alias mv='mv -i'

# GD - Sept 2019
# Linux has renamed ethernet cards depending on their slot and position. This means the name is predictable if you know the 
#   architecture, but unpredictable if running on an unknown machine. These three lines get the name of the network
#   card, bring it up and fetch a DHCP address for it. 
net=`ip -d address | grep "2:" | cut -d: -f2 | head -1`
ifconfig $net up
dhclient

# Now start the wipe script menu
./init-wipe.sh
