#!/bin/sh

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../lib


echo "Starting gpsdatarecv..."
nohup ./gpsdatarecv &
echo "Done."
