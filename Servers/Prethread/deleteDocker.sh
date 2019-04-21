#!/bin/bash
if [ -x "$(command -v docker)" ]; then
    sudo docker container kill serverprethread
    sudo docker container rm serverprethread
    sudo docker image rm serverprethread
else
	echo "docker not found"
fi 	