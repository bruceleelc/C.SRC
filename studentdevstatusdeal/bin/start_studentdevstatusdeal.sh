#!/bin/sh

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../lib


echo "Starting GPSDataRecvier..."
nohup ./studentdevstatusdeal &
echo "Done."
