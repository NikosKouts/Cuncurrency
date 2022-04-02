#!/bin/bash

#Compile Once
gcc -g -Wall main.c -lpthread

for i in $(seq 1 24)
do
    printf "$i "
    (time ./a.out $i) 2>&1 >/dev/null | grep real | awk -F '	' '{print $2}' | awk -F 'm' '{print $1$2}' | sed 's/s//g' | tr "," "."
done
