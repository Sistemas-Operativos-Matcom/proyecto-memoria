#!/usr/bin/bash

mkdir -p build
gcc -o build/main $(find src | grep -P "\.c") -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter
