#!/bin/bash

ofile=pomiar_zad_1.txt

echo "system function" > $ofile
(time ./main_sys testfile tmp) 2>> $ofile
echo "" >> $ofile
echo "library function" >>  $ofile
(time ./main_lib testfile tmp) 2>> $ofile