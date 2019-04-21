server: compile runDaemon

compile:
	@echo "making server files..."
	sudo gcc -o server serverFifo.c
	sudo cp config/config.conf /etc/webserver
	sudo cp server /etc/webserver
	@echo "making server Fifo done!"

runDaemon:
	@echo "starting Daemon"
	sudo cp services/WebServer.service /etc/systemd/system
	sudo systemctl enable WebServer.service
	sudo systemctl start WebServer.service

clean:
	@rm -rf *.o
	@rm -rf server

delete:
	sudo systemctl stop WebServer.service	
	sudo systemctl disable WebServer.service
	sudo rm /etc/systemd/system/WebServer.service
	sudo rm /etc/webserver/config/config.conf
	sudo rm /etc/webserver/server
	sudo systemctl daemon-reload
	sudo systemctl reset-failed

help:
	@echo "run 'sudo make all' to start the server as daemon"
	@echo "run 'sudo make delete' to delete the daemon"
	@echo "run 'sudo make help' to get help"
	@echo "contact ceteam for extra help"