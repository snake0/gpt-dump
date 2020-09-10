#!/bin/bash
make 
sudo insmod lkm.ko
sudo rmmod lkm
dmesg
