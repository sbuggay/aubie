/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdint.h>
#include <inttypes.h>

#include <arpa/inet.h>

#define PORT "10014" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

/*struct packet{
    uint16_t tml;
    uint16_t requestid;
    uint8_t operation;
    //char *string;
} __attribute__((packed));*/

struct __attribute__((packed)) packet{
    uint16_t tml;
    uint16_t requestid;
    uint8_t operation;
    char *message;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 5) {
        fprintf(stderr,"usage: client servername port operation message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

	//Construct packet
	printf("Sending message: %s\n", argv[4]);
    struct packet *test = malloc(sizeof(struct packet));
    test->requestid = 1;  //needs to be reset by the server
	int oper = atoi(argv[3]);
	test->operation = oper; //needs to be changed to cmd line arg input
	//printf("Operation: %d\n", test->operation);
    test->tml = 5 + strlen(argv[4]);  //5 + message length
	//printf("Total message length: %d\n", test->tml);
	test->message = strdup(argv[4]); //Copy argv[2] into struct
	//printf("Message: %s\n", test->message);
	
	printf("(%d|%d|%d|%s)\n", test->tml, test->requestid, test->operation, test->message); //Print test packet
    printf("Total packet size: %d\nLength of string: %d\n", sizeof(*test), strlen(test->message)); //Print size and length

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);
    //printf("sending %d, %d", test->requestid, test->tml);
    write(sockfd, argv[2], sizeof(argv[2]));
    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = read(sockfd, buf, MAXDATASIZE-1)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}