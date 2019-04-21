#!/bin/bash
systemctl stop WebServer.service	
systemctl disable WebServer.service
rm /etc/systemd/system/WebServer.service
rm /etc/webserver/config.conf
rm /etc/webserver/server
systemctl daemon-reload
systemctl reset-failed