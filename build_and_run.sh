#!/usr/bin/env bash

SERVER="pong_server"
CLIENT="pong_client"

cmake --build build/

cd build/

./src/${SERVER} &

./src/${CLIENT} &

./src/${CLIENT} &

wait
