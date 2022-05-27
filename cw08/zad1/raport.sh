#!/bin/bash

rm raport.txt

for i in 1 2 4 8 16
do
    echo "Variant: numbers, size: $i" >> raport.txt
    ./zad $i numbers baboon.ascii.pgm out.pgm >> raport.txt
    echo >> raport.txt
done

for i in 1 2 4 8 16
do
    echo "Variant: block, size: $i" >> raport.txt
    ./zad $i block baboon.ascii.pgm out.pgm >> raport.txt
    echo >> raport.txt
done