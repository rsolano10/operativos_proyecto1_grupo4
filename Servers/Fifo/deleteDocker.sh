#!/bin/bash

if [ -x "$(command -v docker)" ]; then
    sudo docker container kill serverfifo
    sudo docker container rm serverfifo 
    sudo docker image rm serverfifo  
else
	echo "docker not found"
fi 