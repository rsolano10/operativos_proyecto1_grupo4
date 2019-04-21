#!/bin/bash
cp /usr/Servers/config/config.conf /etc/webserver
cp /usr/Servers/server /etc/webserver
cp /usr/Servers/services/WebServer.service /etc/systemd/system
systemctl enable WebServer.service
systemctl start WebServer.service