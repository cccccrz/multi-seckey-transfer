#!/bin/bash
PID=`ps -ef | grep cltMain | grep -v grep | awk '{print$2}'`
if [ -n $PID ]
then
	kill -9 $PID
fi