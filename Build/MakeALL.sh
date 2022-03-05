#!/bin/bash

Ver="Beta2.1.1"
Date=`date`

buildSet=(snapCterm-40col snapCterm-80col)
buildNetType=(if1-rs232 p-rs232 p3-rs232 snet)

mkdir bin
mkdir src
mkdir pub


if [[ -d ./pub/$Ver/ ]]
then
    echo "We already got one "
    rm -R ./pub/$Ver/
    mkdir ./pub/$Ver/
else 
    mkdir ./pub/$Ver/
    
fi
printf "snapCterm %s Built on %s" "$Ver" "$Date" > ./pub/$Ver/ReadMe.txt



echo "Cleaning up"
pwd
rm ./bin/*
rm ./src/*

for build in "${buildSet[@]}" ;do

    for netType in "${buildNetType[@]}" ;do
    mfile="Makefile.$build-$netType"
    echo $mfile
    make -f $mfile >& ./bin/$build-$netType-build.log
    cat ./bin/$build-$netType-build.log | grep Error

    if [[ -d ./pub/$Ver/$build-$netType/ ]]
    then
        echo " We already got one "
        rm -R ./pub/$Ver/$build-$netType/
    fi

    echo "produce tap and dsk files"
    pushd bin
    pwd
    cp *.tap ../pub/$Ver/$Ver-$build-$netType.tap
    cp *.dsk ../pub/$Ver/$Ver-$build-$netType.dsk
    mv *.tap ../pub/$Ver/$build-$netType.tap
    mv *.dsk ../pub/$Ver/$build-$netType.dsk
    popd

    echo cp -r ./bin/ ./pub/$Ver/$Ver-$build-$netType/
    cp -r ./bin/ ./pub/$Ver/$Ver-$build-$netType/
    
    echo "Cleaning up"
    pwd
    rm ./bin/*
    rm ./src/*
    done

done
