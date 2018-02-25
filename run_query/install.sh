#!/bin/bash

# builds and installs on PALMETTO

module load gcc mpich python

make -j

if [ -z "$MOLIERE_HOME" ]; then
  echo "ADDING MOLIERE_HOME ENV VAR TO BASHRC"
  echo "" >> ~/.bashrc
  echo "# ADDED BY MOLIERE:" >> ~/.bashrc
  echo "export MOLIERE_HOME=$PWD" >> ~/.bashrc
  echo "source ~/.bashrc to get started now"
fi
