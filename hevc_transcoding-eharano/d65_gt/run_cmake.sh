#!/bin/bash

mkdir build
cd build

cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release ..

make
