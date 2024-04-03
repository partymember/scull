#!/bin/sh
group="staff"
device="scull"
mode=664
module="scull"

/sbin/rmmod ./$module.ko $* || exit 1

rm -f /dev/${device}[0-3]

