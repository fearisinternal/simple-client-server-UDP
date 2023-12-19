#!/bin/bash
cmake -S . -B ./build

./build/server/server &
./build/client/client ./tests/test_video.mp4
killall server
