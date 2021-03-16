#!/bin/bash
clear
sudo dmesg --clear
make 
sudo rmmod lkm
sudo insmod lkm.ko
sudo rmmod lkm
dmesg > ptdump.txt
