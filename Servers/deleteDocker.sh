#!/bin/bash

## Servers compiling and docker running##
echo "delete server Fifo..."
cd  Fifo
sudo bash deleteDocker.sh
cd ..
echo "delete server Fork..."
cd Fork
sudo bash deleteDocker.sh
cd ..
echo "delete server Thread..."
cd Thread
sudo bash deleteDocker.sh
cd ..
echo "delete server PreThread..."
cd Prethread
sudo bash deleteDocker.sh
cd ..
echo "delete server Prefork..."
cd Prefork
sudo bash deleteDocker.sh
cd ..
