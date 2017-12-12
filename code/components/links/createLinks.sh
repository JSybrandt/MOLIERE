#!/bin/bash
# This file makes links for all bin execs in the above dir
shopt -s extglob
rm -- !(createLinks.sh)
find .. -path '*/bin/*' -exec ln -sf {} \;

PROJ_HOME=$(pwd | grep -o .*moliere)
ln -s $PROJ_HOME/tools/plda/mpi_lda
ln -s $PROJ_HOME/tools/plda/view_model.py


