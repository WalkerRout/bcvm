#!/bin/bash

PROJ_NAME=bcvm

cd build/ # important

if [ "$1" == "reset" ]
then
  rm -r * # !!!!!!
  cmake ..
  make "$PROJ_NAME"_run; src/"$PROJ_NAME"_run
elif [ "$1" == "build" ]
then
  cmake ..
else
  make "$PROJ_NAME"_run; src/"$PROJ_NAME"_run
fi

cd ..
