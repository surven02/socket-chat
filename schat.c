//Surya Venkatraman sv2wx
//Include necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

//Define frequently used values
#define MAX_MSG_LEN 4096
#define POLL_TIMEOUT 60000

//main function accepting command-line arguments
int main(int argc, char* argv[]){

//initialize variables
//use a boolean mask to determine whether it is in server mode or client mode
	int is_server = 0;
	int is_client = 0;
	int port;
	int sockfd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clientSize;
	char buffer[MAX_MSG_LEN];
//use the poll libary to determine where input stream is coming from
	struct pollfd pollfds[2];
	int fileDescriptors = 2;
	char* ip_address;
//if there is one argument (./a.out), run on server mode with a random port number
	if (argc == 1) {
		is_server = 1;
		port = 0xc000|(random()&0x3fff);
		system("host $HOSTNAME");
		printf("Listening on port %d\n", port);
//if the port number was given as an argument, use the given port number and run on server mode
	} else if (argc == 2) {
		is_server = 1;
		port = atoi(argv[1]);
		system("host $HOSTNAME");
		printf("Listening on port %d\n", port);
//if the command-line arguments have both the address and the port number, run on client mode
	} else if (argc == 3) {
		is_client = 1;
		ip_address = argv[1];
		port = atoi(argv[2]);
	} else {
//ERROR CHECKING
		return 1;
	}
//Create socket from socket stream
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
//ERROR CHECKING
	if (sockfd < 0) {
		return 1;
	}
//if in server mode, listen for any connections on the specified IP address, accept one and communicate with client

	if (is_server) {
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
//ERROR CHECKING
	serv_addr.sin_port = htons(port);
		if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			return 1;
		}

		listen(sockfd, 1);
//create new socklen_t object to calculate size of client address object and accept to not read beyond end of client address structure
		clientSize = sizeof(cli_addr);
		int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clientSize);
//ERROR CHECKING
	if(newsockfd < 0) {
			return 1;
		}
//close the socket
		close(sockfd);
		sockfd = newsockfd;
//client will connect to given IP and port and communicate to server over the socket using htons()
	} else {
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);
//ERROR CHECKING
		if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
			return 1;
		}
//ERROR CHECKING
		if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			return 1;
		}
	}
//poll() listens to both sides and waits for input, sockfd and STDIN_FILENO
//two arrays struct pollfd are made to continuously monitor information and
//trigger POLLIN when there is data to be read.
	pollfds[0].fd = sockfd;
	pollfds[0].events = POLLIN;
	pollfds[1].fd = STDIN_FILENO;
	pollfds[1].events = POLLIN;
//in an infinite loop (until CTRL+D is pressed or the TIMEOUT value is reached)
//, poll() is called to check whether an fd has an entry event.
	while (1) {
		int res = poll(pollfds, fileDescriptors, POLL_TIMEOUT);
//ERROR CHECKING
	if (res == -1) {
			return 1;
		} else if (res == 0) {
			continue;
		}

		if (pollfds[1].revents & POLLIN) {
//STDIN_FILENO reads data using(read) and writes it to socket repeatedly
			ssize_t len = read(STDIN_FILENO, buffer, MAX_MSG_LEN);

		if (len == -1) {
				return 1;
			} else if (len == 0) {
				break;
			} else {
//if sockfd has something to listen to, it writes the data to STDOUT below
//EOF is CTRL+D
				if (write(sockfd, buffer, len) == -1) {
					return 1;
				}
			}
		}

		if(pollfds[0].revents & POLLIN) {
			ssize_t len = read(sockfd, buffer, MAX_MSG_LEN);
//ERROR CHECKING
			if (len == -1) {
				return 1;
			} else if (len == 0) {
				break;
			} else {
//ERROR CHECKING
			if (write(STDOUT_FILENO, buffer, len) == -1) {
					return 1;
				}
			}
		}
	}
//close the socket
	close(sockfd);
	return 0;
}
