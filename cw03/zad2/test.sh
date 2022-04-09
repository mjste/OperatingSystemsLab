#!/bin/bash

echo "Raport" > raport.txt

for width in 0.1 0.01 0.001 0.0001 0.00001 0.000001 0.0000001 0.00000001 0.000000001 0.0000000001
do
    for n in {1..6}
    do
        echo "width=$width n=$n" >> raport.txt
        (time (./main $width $n) 2>&1) >> raport.txt
        echo -e "\n" >> raport.txt

    done
done