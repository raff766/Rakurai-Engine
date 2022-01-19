#!/bin/sh

numOfJobs=6

if [ ! -d "build" ] 
then
    mkdir build
fi

cmake -S . -B build
cmake --build build -j $numOfJobs
