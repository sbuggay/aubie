// Group 4
// ClientTCP.c
// compile with gcc ClientTCP.c -o ClientTCP
// run with ./ClientTCP [hostname] [port] [operation] [string]

// includes
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
#include <time.h>

#define MAXDATASIZE 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 5) { // check for correct args
        fprintf(stderr,"usage: client [hostname] [port] [operation] [message]\n");
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

    printf("Sending message: %s\n", argv[4]);

    // setup variables
	uint16_t tml;
	uint16_t requestid = 1; // requestid can be whatever
	uint8_t operation = atoi(argv[3]);

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    //create packet
    uint8_t *buf = malloc(5 + strlen(argv[4])); // allocate space for packet
    uint8_t *pos = buf; //point pointer
    *(uint16_t*)pos = 5 + strlen(argv[4]);
    pos += sizeof(uint16_t);
    *(uint16_t*)pos = requestid;
    pos += sizeof(uint16_t);
    *(u_int8_t*)pos = operation;
    pos += sizeof(u_int8_t);
    strcpy( pos, argv[4] );

    time_t start,end;
    time (&start);

    // send packet
    write(sockfd, buf, 5 + strlen(argv[4]));

    freeaddrinfo(servinfo); // all done with this structure
	
	uint8_t *in = malloc(MAXDATASIZE);
    if ((numbytes = read(sockfd, in, MAXDATASIZE-1)) == -1) {
        perror("recv");
        exit(1);
    }

    // disemvowel operation
	if (operation == 170) {
		uint8_t *inp = in;
		uint16_t tml, rid;
        //unpack (h,h,s) packet
		memcpy(&tml, inp, sizeof(uint16_t)); //copy first 
		inp += sizeof(uint16_t);
		memcpy(&rid, inp, sizeof(uint16_t));
		inp += sizeof(uint16_t);
		char *inbuf = malloc(tml - 4);
		strcpy(inbuf, inp);
		inbuf[tml - 4] = '\0'; // set null terminator
		printf("client: received tml is '%hu'\n",tml);
		printf("client: received rid is '%hu'\n",rid);
		printf("client: received disemvoweled message '%s'\n",inbuf);
	} 
    // vowel count operation
	else { 
		uint8_t *inp = in;
		uint16_t tml, rid, vowels;
        // unpack (h,h,h) uint16_t
		memcpy(&tml, inp, sizeof(u_short));
		inp += sizeof(uint16_t);
		memcpy(&rid, inp, sizeof(uint16_t));
		inp += sizeof(uint16_t);
		memcpy(&vowels, inp, sizeof(uint16_t));
		printf("client: received tml is '%hu'\n",tml);
		printf("client: received rid is '%hu'\n",rid);
		printf("client: received vowel length '%hu'\n",vowels);
	}
	
    close(sockfd);
    return 0;
}

