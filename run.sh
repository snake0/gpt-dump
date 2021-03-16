#!/bin/bash
sudo dmesg --clear
make 
sudo rmmod lkm
sudo insmod lkm.ko
sudo rmmod lkm
dmesg
