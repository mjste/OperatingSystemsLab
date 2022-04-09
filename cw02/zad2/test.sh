#!/bin/bash

ofile=pomiar_zad_2.txt

echo "system function" >  $ofile
(time ./main_sys a testfile 1>&2) 2>> $ofile 
echo "" >> $ofile
echo "library function" >>  $ofile
(time ./main_lib a testfile 1>&2) 2>> $ofile 