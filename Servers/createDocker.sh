#!/bin/bash

## Install docker ##
echo "installing docker..."
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

## Build Essentials ##
echo "building essentials..."
sudo apt -y install build-essential

## verify containers ##
if [ "$(docker ps -q -f name=serverfifo)" ]; then
    echo "server fifo already created"
else 
    echo "starting server Fifo..."
    cd  Fifo/Servers
    sudo make server
    cd ..
    sudo bash createDocker.sh
    cd ..
fi

if [ "$(docker ps -q -f name=serverfork)" ]; then
    echo "server fork already created"
else
    echo "starting server Fork..."
    cd Fork/Servers
    sudo make server
    cd ..
    sudo bash createDocker.sh
    cd ..
fi

if [ "$(docker ps -q -f name=serverthread)" ]; then
    echo "server thread already created"
else
    echo "starting server Thread..."
    cd Thread/Servers
    sudo make server
    cd ..
    sudo bash createDocker.sh
    cd ..
fi

if [ "$(docker ps -q -f name=serverprethread)" ]; then
    echo "server prethread already created"
else
    echo "starting server Prethread..."
    cd Prethread/Servers
    sudo make server
    cd ..
    sudo bash createDocker.sh
    cd ..
fi

if [ "$(docker ps -q -f name=serverprefork)" ]; then
    echo "server prefork already created"
else
    echo "starting server Prefork..."
    cd Prefork/Servers
    sudo make server
    cd ..
    sudo bash createDocker.sh
    cd ..
fi
