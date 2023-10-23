#!/bin/bash
cmake -S . -B ./build

./build/server/server &
./build/client/client %path_to_file%
killall server
