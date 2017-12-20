#!/bin/sh
mkdir /home/abk/devstatusdeal
cd /home/svn/C.SRC/devstatusdeal/
make clean;make
cp bin/* /home/abk/devstatusdeal/

mkdir /home/abk/gpsalarmdeal
cd /home/svn/C.SRC/gpsalarmdeal/
make clean;make
cp bin/* /home/abk/gpsalarmdeal/

mkdir /home/abk/gpsdata
cd /home/svn/C.SRC/gpsdata/
make clean;make
cp bin/* /home/abk/gpsdata/

mkdir /home/abk/gpslocationdeal
cd /home/svn/C.SRC/gpslocationdeal/
make clean;make
cp bin/* /home/abk/gpslocationdeal/

mkdir /home/abk/studentdevstatusdeal
cd /home/svn/C.SRC/studentdevstatusdeal/
make clean;make
cp bin/* /home/abk/studentdevstatusdeal/

mkdir /home/abk/studentgpsdatarecv
cd /home/svn/C.SRC/studentgpsdatarecv/
make clean;make
cp bin/* /home/abk/studentgpsdatarecv/

mkdir /home/abk/studentgpslocationdeal
cd /home/svn/C.SRC/studentgpslocationdeal/
make clean;make
cp bin/* /home/abk/studentgpslocationdeal/

cd /home/svn/C.SRC/
cp killall.sh /home/abk/
cp startall.sh /home/abk
cp -r lib /home/abk/