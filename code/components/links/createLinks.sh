#!/bin/bash
# This file makes links for all bin execs in the above dir
find .. -path '*/bin/*' -exec ln -sf {} \;


