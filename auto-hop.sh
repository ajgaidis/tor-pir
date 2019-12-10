#!/bin/bash

## This script is used to query the plain server over and over again via Tor

for x in {0..30}
do
  tor &      # Start Tor and put it in the background
  PID=$!     # Get the pid of the Tor process running the background
  sleep 7    # Sleep for 7 seconds to allow Tor to setup
  torify ./bin/client --host=3.14.27.183 --port=8080 --ele_num=65536 --ele_size=512  # Query
  kill $PID  # Kill Tor so that each time through the loop we get a fresh circuit
done