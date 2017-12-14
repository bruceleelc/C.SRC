#!/bin/sh

server_scripts=$(cd "$(dirname "$0")"; pwd)

cd $server_scripts


while true
do
	sleep 10
	proc_count=`ps -ef|grep gpsalarmdeal$|grep -v grep|wc -l`
	if [ $proc_count -eq 0 ]; then
	    ./start_gpsalarmdeal.sh
	fi
	
	
done
