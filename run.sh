#!/bin/bash
clear
sudo dmesg --clear
make 
sudo rmmod lkm
sudo insmod lkm.ko
sudo rmmod lkm
dmesg --notime > ptdump.txt
