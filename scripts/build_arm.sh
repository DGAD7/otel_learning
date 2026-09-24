#!/bin/bash

set -e
set -x

cmake -B build -S .

cmake --build build
