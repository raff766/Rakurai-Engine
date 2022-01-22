#!/bin/sh

numOfJobs=6

if [ ! -d "build" ] 
then
    mkdir build
fi

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S . -B build
cmake --build build -j $numOfJobs
