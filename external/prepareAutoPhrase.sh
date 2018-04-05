#!/bin/bash

if [ -z "$MOLIERE_HOME" ];then
  echo "FAILED TO INSTALL MOLIERE_HOME"
  exit 1
fi

cd $MOLIERE_HOME
cd external/AutoPhrase

# defines LARGE in header
#sed -i 's/^\/\/ #define LARGE$/^#define LARGE$/g' \
       #src/utils/parameters.h

# removes word limit per abstract in tokenizer
#sed -i 's/if\s*(\s*loadCount\s*>\s*100)/if(false)/g'\
       #tools/tokenizer/src/Tokenizer.java


