#!/usr/bin/env bash

#Build IF1

rm -r ../snapCterm-if1
mkdir ../snapCterm-if1
cp Makefile.snapCterm-if1 ../snapCterm-if1/Makefile.snapCterm-if1
cd ../snapCterm-if1
make -f ../snapCterm-if1/Makefile.snapCterm-if1
cp ../snapCterm-if1/sCtIF1.tap ../Build/snapCterm-IF1.tap
cd ../Build

#Build 128K Plus
rm -r ../snapCterm-p
mkdir ../snapCterm-p
cp Makefile.snapCterm-p ../snapCterm-p/Makefile.snapCterm-p
cd ../snapCterm-p
make -f ../snapCterm-p/Makefile.snapCterm-p
cp ../snapCterm-p/sCtp.tap ../Build/snapCterm-plus.tap
cd ../Build

#Build Plus3 disk
rm -r ../snapCterm-p3
mkdir ../snapCterm-p3
cp Makefile.snapCterm-p3 ../snapCterm-p3/Makefile.snapCterm-p3
cd ../snapCterm-p3
make -f ../snapCterm-p3/Makefile.snapCterm-p3
cp ../snapCterm-p3/sCtp3.dsk ../Build/snapCterm-plus3.dsk
cd ../Build
