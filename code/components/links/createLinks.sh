#!/bin/bash
# This file makes links for all bin execs in the above dir
shopt -s extglob
rm -- !(createLinks.sh)
find .. -path '*/bin/*' -exec ln -sf {} \;


