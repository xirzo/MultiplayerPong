#!/usr/bin/env bash

SERVER="pong_server"
CLIENT="pong_client"

cmake --build build/

cd build/

./bin/${SERVER} &

./bin/${CLIENT} &

./bin/${CLIENT} &

wait
