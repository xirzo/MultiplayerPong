#!/usr/bin/env bash

SERVER="pong_server"
CLIENT="pong_client"

cmake --build build/ || { echo "Build failed!"; exit 1; }

cd build/ || exit 1

PIDS=()

cleanup() {
    echo -e "\nShutting down server and clients..."
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null
    done
    
    trap - SIGINT SIGTERM EXIT
    exit 0
}

trap cleanup SIGINT SIGTERM EXIT

echo "Starting $SERVER..."
./src/${SERVER} &
PIDS+=($!)

echo "Starting $CLIENT 1..."
./src/${CLIENT} &
PIDS+=($!)

echo "Starting $CLIENT 2..."
./src/${CLIENT} &
PIDS+=($!)

echo "All processes started. Press Ctrl+C to stop."

wait
