#!/bin/bash
if [ -x "$(command -v docker)" ]; then
    sudo docker container kill serverfork
    sudo docker container rm serverfork 
    sudo docker image rm serverfork  
else
	echo "docker not found"
fi 