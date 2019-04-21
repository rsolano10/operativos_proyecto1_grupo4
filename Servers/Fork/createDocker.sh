#!/bin/bash
    IP=`hostname  -I | cut -f1 -d' '`
    HOSTIP=8003
    DOCKERIP=8003

    ## Install docker ##
    if [ -x "$(command -v docker)" ]; then
        echo "Docker installed"
    else
        sudo apt-get update
        sudo apt-get remove docker docker-engine docker.io
        sudo apt install apt-transport-https ca-certificates curl software-properties-common
        curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
        sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu bionic stable"
        sudo apt update
        apt-cache policy docker-ce
        sudo apt install docker-ce
    fi

    ## make Docker ##
    sudo docker build --rm -t serverfork .
    sudo docker run -itd --privileged -p $IP:$HOSTIP:$DOCKERIP --name=serverfork serverfork

    ## Run Server inside docker ##
    sudo docker exec -it serverfork /bin/bash -c /usr/Servers/runServer.sh

    ## go to docker container ##
    if [ "$1" == "-i" ]; then
        sudo docker exec -t -i serverfork /bin/bash
    fi
