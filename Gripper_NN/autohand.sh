#!/bin/bash
while true; do
python camera.py
./read_angles
read -p "Quit?[q]" choice
case $choice in
	[Qq]* ) break;;
	* ) continue;;
esac
done
