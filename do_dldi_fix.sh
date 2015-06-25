#!/bin/bash

path_to_dlditool=/home/fl4shk/devkitPro/devkitARM/bin

for a in *.gba;
do
	${path_to_dlditool}/dlditool ezsd.dldi $a
done
