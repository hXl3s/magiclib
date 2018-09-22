#!/bin/bash

rm -rf build/
mkdir build/
cd build

cmake .. && make -j9 && make test
