#!/bin/bash
make
for a in *.gba;
do
	dlditool ezsd.dldi $a
done
