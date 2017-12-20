#!/bin/sh

cd /home/abk/devstatusdeal/
./start_devstatusdeal.sh
cd /home/abk/gpsalarmdeal/
./start_gpsalarmdeal.sh
cd /home/abk/gpsdata/
./start_GPSDataRecvier.sh
cd /home/abk/gpslocationdeal/bin
./start_gpslocationdeal.sh
cd /home/abk/studentdevstatusdeal/
./start_studentdevstatusdeal.sh
cd /home/abk/studentgpsdatarecv/
./start_studentgpsdatarecv.sh
cd /home/abk/studentgpslocationdeal/
./start_studentgpslocationdeal.sh

