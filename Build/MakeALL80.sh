#!/usr/bin/env bash

Ver="Beta2.1.1"
# Build using Z88DK 2.1 & SpectraNet Libraries

#Build RS232 IF1
pwd
rm ../src/*.o
rm -r ../snapCterm-80col-if1-rs232
mkdir ../snapCterm-80col-if1-rs232
cp Makefile.snapCterm-80col-if1-rs232 ../snapCterm-80col-if1-rs232/Makefile.snapCterm-80col-if1-rs232
cd ../snapCterm-80col-if1-rs232
make -f ../snapCterm-80col-if1-rs232/Makefile.snapCterm-80col-if1-rs232
cp ../snapCterm-80col-if1-rs232/sCtIF1rs.tap ../Build/snapCterm-80col-IF1-rs232_$Ver.tap
cd ../Build

#Build 128K RS232 (Plus)
rm ../src/*.o
rm -r ../snapCterm-80col-p-rs232
mkdir ../snapCterm-80col-p-rs232
cp Makefile.snapCterm-80col-p-rs232 ../snapCterm-80col-p-rs232/Makefile.snapCterm-80col-p-rs232
cd ../snapCterm-80col-p-rs232
make -f ../snapCterm-80col-p-rs232/Makefile.snapCterm-80col-p-rs232
cp ../snapCterm-80col-p-rs232/sCtprs.tap ../Build/snapCterm-80col-plus-rs232_$Ver.tap
cd ../Build

#Build 128K SNET Plus
rm ../src/*.o
rm -r ../snapCterm-80col-snet
mkdir ../snapCterm-80col-snet
cp Makefile.snapCterm-80col-snet ../snapCterm-80col-snet/Makefile.snapCterm-80col-snet
cd ../snapCterm-80col-snet
make -f ../snapCterm-80col-snet/Makefile.snapCterm-80col-snet
cp ../snapCterm-80col-snet/sCtsn.tap ../Build/snapCterm-80col-snet_$Ver.tap
cd ../Build

#Build Plus3 RS232 disk
rm ../src/*.o
rm -r ../snapCterm-80col-p3-rs232
mkdir ../snapCterm-80col-p3-rs232
cp Makefile.snapCterm-80col-p3-rs232 ../snapCterm-80col-p3-rs232/Makefile.snapCterm-80col-p3-rs232
cd ../snapCterm-80col-p3-rs232
make -f ../snapCterm-80col-p3-rs232/Makefile.snapCterm-80col-p3-rs232
cp ../snapCterm-80col-p3-rs232/sCtp3rs.dsk ../Build/snapCterm-80col-plus3-rs232_$Ver.dsk
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