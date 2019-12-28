#include <stdio.h> 
#include <strings.h>
#include <string.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <errno.h>

void help();

int main(int argc, char **argv) {
	// get options
	int opt;
	int PORT = getenv("L2PORT") != NULL? atoi(getenv("L2PORT")) : -1;
	char *ip = getenv("L2ADDR") != NULL? getenv("L2ADDR") : "127.0.0.1";
	while ((opt = getopt(argc, argv, "p:hva:")) != -1) {
		switch(opt) {
			case 'p':
				PORT = atoi(optarg);
				break;
			case 'v':
				printf("Version: lab2 version 4.0\n");
				exit(0);
			case 'h':
				help();
				exit(0);
			case 'a':
				ip = optarg;
				break;
		}
	}
	// remind setting PORT
	if (PORT == -1) {
		printf("PORT is not set/n");
		exit(EXIT_FAILURE);
	}

	char buffer[1024]; 
	char *request = (char*) calloc(100, sizeof(char));
	strcpy(request, argv[optind]);
	if (argv[optind + 1] != NULL) {
		strcat(request, " ");
		strcat(request, argv[optind + 1]);
	}
	int sockfd; 
	struct sockaddr_in servaddr;
	
	// clear servaddr 
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(PORT); 
	servaddr.sin_family = AF_INET; 
	
	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("ERROR %d: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}; 

	// connect to server 
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
		printf("ERROR %d: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	} 

	// connect stores the peers IP and port 
	send(sockfd, request, strlen(request), 0);
	
	// waiting for response 
	bzero(buffer, sizeof(buffer));
	read(sockfd, buffer, 1024);
	printf("%s\n", buffer); 

	// close the descriptor 
	close(sockfd);
}

// Print help
void help() {
    printf("Usage: ./client -a [IP] -p [PORT] -v -h\n");
    printf("\n");
    printf("  -a\t The address on which the server listens and to which the client connects.\n");
    printf("  -p\t The port on which the server listens and to which the client connects.\n");
    printf("  -v\t version\n");
    printf("  -h\t helpl.\n");
}


