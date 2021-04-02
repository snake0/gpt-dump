#!/bin/bash
clear
sudo dmesg --clear
make

sudo insmod lkm.ko
dmesg --notime > ptdump.txt
sudo dmesg --clear

sudo rmmod lkm.ko
cat ptdump.txt
