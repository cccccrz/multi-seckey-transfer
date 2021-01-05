#!/bin/bash
PID=`ps -ef | grep cltMain | grep -v grep | awk '{print$2}'`
if [ -n $PID ]
then
	kill -9 $PID
<<<<<<< HEAD
fi  
=======
fi  
>>>>>>> cff4158363868422e24baf3def809f2871053688
