#!/bin/bash
if [ -x "$(command -v docker)" ]; then
    sudo docker container kill serverprefork
    sudo docker container rm serverprefork
    sudo docker image rm serverprefork
else
	echo "docker not found"
fi 	