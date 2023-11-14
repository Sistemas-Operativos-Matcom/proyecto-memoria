#!/bin/bash

mkdir -p build
gcc -o build/main $(find src | grep -P "\.c") -lm -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter
