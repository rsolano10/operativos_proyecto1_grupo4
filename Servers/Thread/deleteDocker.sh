#!/bin/bash
if [ -x "$(command -v docker)" ]; then
    sudo docker container kill serverthread
    sudo docker container rm serverthread
    sudo docker image rm serverthread
else
	echo "docker not found"
fi 	