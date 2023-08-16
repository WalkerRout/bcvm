#!/bin/bash

PROJ_NAME=bcvm

cd build/

if [ "$1" == "reset" ]
then
  rm -r *
  cmake ..
  make "$PROJ_NAME"_run; src/"$PROJ_NAME"_run
else
  make "$PROJ_NAME"_run; src/"$PROJ_NAME"_run
fi

cd ..
