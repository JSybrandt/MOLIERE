#!/bin/bash

pushd $1
for f in $(ls); do
  cd $f
  for h in $(ls); do
    mv $h ../$h
  done
  cd ..
  rmdir $f
done
popd
