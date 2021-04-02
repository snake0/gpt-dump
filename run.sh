#!/bin/bash
clear
sudo dmesg --clear
make 
sudo insmod lkm.ko
dmesg --notime > ptdump.txt
sudo dmesg --clear

cat ptdump.txt
