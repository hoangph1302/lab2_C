#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <sys/stat.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include "servlib.h"
#include <signal.h>
#include <errno.h>
#include <netdb.h>

int main(int argc, char **argv) {
	char buffer[100];
	char *message; 
	int sockfd, connfd;
	struct sockaddr_in serverAddress, clientAddress;
	int cliAddrLen = sizeof(clientAddress);
	int daemonMode = 0;
	bzero(&serverAddress, sizeof(serverAddress));
	
	// Get options
	int opt;
	int portNumber = getenv("L2PORT") != NULL? atoi(getenv("L2PORT")) : -1;
	int waitTime = 0;
	char* logfile = (char *) calloc(20, sizeof(char));
	logfile = getenv("L2LOGFILE") != NULL ? getenv("L2LOGFILE") : "/tmp/lab2.log";
	char *ip = getenv("L2ADDR") != NULL? getenv("L2ADDR") : "127.0.0.1";
	int mode = PUBLIC;
	while ((opt = getopt(argc, argv, "p:w:vhol:da:")) != -1) {
		switch(opt) {
			case 'p':
				portNumber = atoi(optarg);
				break;
			case 'w':
				waitTime = atoi(optarg);
				break;
			case 'v':
				printf("Version: lab2 version 4.0\n");
				exit(0);
			case 'h':
				help();
				exit(0);
			case 'l':
				logfile = optarg;
				break;
			case 'd':
				daemonMode = 1;
				break;
			case 'a':
				ip = optarg;
				break;
			case 'o':
				mode = PRIVATE;
				break;
			case '?':
				printf("unknown option: %c\n", optopt);
				break;

		}
	}
	
	// Create logfile
	createLogfile(logfile);
	
	// Remind setting PORT
	if (portNumber == -1) {
		printf("[!!!]PORT is not set\n Please enter portnumber or Use the command: export L2PORT=[port]\n");
		exit(EXIT_FAILURE);
	}

	// mode daemon
	if (daemonMode == 1) {
		modeDaemon();
	}

	database(mode);		// Create database
	atexit(deleteDatabase);		// Remove database when server stops working

	// Create a TCP Socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		show(messError(errno));
		exit(EXIT_FAILURE);
	}

    // assign IP, PORT
    serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ip);
	serverAddress.sin_port = htons(portNumber);


    // Binding newly created socket to given IP and verification
	if (bind(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		show(messError(h_errno));
		exit(EXIT_FAILURE);
	};

    timeListen();


	if (listen(sockfd, 5) < 0) {
		show(messError(errno));
		exit(EXIT_FAILURE);
	}
	else
        printf("Server is listening at Port: %d \n", portNumber);


    // Handle signals with sigaction
	struct sigaction act;
	bzero(&act, sizeof(act));
	act.sa_handler = signalHandler;
	act.sa_flags = SA_RESTART;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	
	// Ignore SIGCHLD
	ignoreSIGCHLD();
	
	pid_t pid;
	while(1) {
        // Accept the data packet from client and verification
		connfd = accept(sockfd, (struct sockaddr*)&clientAddress, (socklen_t*)&cliAddrLen);
        char* clientIP = (char *) inet_ntoa(clientAddress.sin_addr);

		if (connfd<0){
           printf("server acccept failed...\n");
            exit(EXIT_FAILURE);
		}
		else {
            printf("server acccept the client IP: %s\n", clientIP);
        }


		//Receive message from server
		memset(&buffer, '\0', sizeof(buffer));
		read(connfd, buffer, 1024);
		
		// Create a new proccess
		pid = fork();
		if (pid < 0) {
			show(messError(errno));
			exit(EXIT_FAILURE);
		} else if (pid == 0) {
			show(request(buffer));
			sleep(waitTime);
			message = handleRequest(buffer, clientIP);
			// send the response
			send(connfd, message, strlen(message), 0);
			show(response(buffer));
			exit(EXIT_FAILURE);
		}
		else {
			increaseNumberServed();
		}
	}
	close(sockfd);
	return 0;
}

