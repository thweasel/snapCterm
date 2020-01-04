#!/usr/bin/env bash

Ver="Beta2.0.4"

#Build RS232 IF1
pwd
rm ../src/*.o
rm -r ../snapCterm-if1-rs232
mkdir ../snapCterm-if1-rs232
cp Makefile.snapCterm-if1-rs232 ../snapCterm-if1-rs232/Makefile.snapCterm-if1-rs232
cd ../snapCterm-if1-rs232
make -f ../snapCterm-if1-rs232/Makefile.snapCterm-if1-rs232
cp ../snapCterm-if1-rs232/sCtIF1rs.tap ../Build/snapCterm-IF1-rs232_$Ver.tap
cd ../Build

#Build 128K RS232 (Plus)
rm ../src/*.o
rm -r ../snapCterm-p-rs232
mkdir ../snapCterm-p-rs232
cp Makefile.snapCterm-p-rs232 ../snapCterm-p-rs232/Makefile.snapCterm-p-rs232
cd ../snapCterm-p-rs232
make -f ../snapCterm-p-rs232/Makefile.snapCterm-p-rs232
cp ../snapCterm-p-rs232/sCtprs.tap ../Build/snapCterm-plus-rs232_$Ver.tap
cd ../Build

#Build 128K SNET Plus
rm ../src/*.o
rm -r ../snapCterm-snet
mkdir ../snapCterm-snet
cp Makefile.snapCterm-snet ../snapCterm-snet/Makefile.snapCterm-snet
cd ../snapCterm-snet
make -f ../snapCterm-snet/Makefile.snapCterm-snet
cp ../snapCterm-snet/sCtsn.tap ../Build/snapCterm-snet_$Ver.tap
cd ../Build

#Build Plus3 RS232 disk
rm ../src/*.o
rm -r ../snapCterm-p3-rs232
mkdir ../snapCterm-p3-rs232
cp Makefile.snapCterm-p3-rs232 ../snapCterm-p3-rs232/Makefile.snapCterm-p3-rs232
cd ../snapCterm-p3-rs232
make -f ../snapCterm-p3-rs232/Makefile.snapCterm-p3-rs232
cp ../snapCterm-p3-rs232/sCtp3rs.dsk ../Build/snapCterm-plus3-rs232_$Ver.dsk
cd ../Build

#Build Plus3 SNET disk  -- Impossible configuration at this time
#rm ../src/*.o
#rm -r ../snapCterm-p3-snet
#mkdir ../snapCterm-p3-snet
#cp Makefile.snapCterm-p3-snet ../snapCterm-p3-snet/Makefile.snapCterm-p3-snet
#cd ../snapCterm-p3-snet
#make -f ../snapCterm-p3-snet/Makefile.snapCterm-p3-snet
#cp ../snapCterm-p3-snet/sCtp3sn.dsk ../Build/snapCterm-plus3-snet.dsk
#cd ../Build