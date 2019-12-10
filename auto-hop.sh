#!/bin/bash

for x in {0..30}
do
  tor &
  PID=$!
  sleep 7
  torify ./bin/client --host=3.14.27.183 --port=8080 --ele_num=65536 --ele_size=512
  kill $PID
done