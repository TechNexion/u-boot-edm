#!/bin/bash

if [ "$1" == "-h" ] || [ -z $1 ] ; then
	echo "Usage: mkbootscr.sh <script_file>"
else
	mkimage -A arm -O linux -T script -C none -n "TN Boot Script" -d $1 $1.uimg
fi

