// Group 4
// ClientTCP.c
// compile with gcc ClientTCP.c -o ClientTCP
// run with ./ClientTCP [hostname] [port] [operation] [string]

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
    //set up some variables
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
	uint8_t operation = atoi(argv[3]); // set operation

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    //create packet
    uint8_t *buf = malloc(5 + strlen(argv[4])); // allocate space for packet
    uint8_t *pos = buf; //point pointer
    *(uint16_t*)pos = 5 + strlen(argv[4]); //set first 2 bytes to total message length (5 + strlen(message))
    pos += sizeof(uint16_t); // move pointer 2 bytes over
    *(uint16_t*)pos = requestid; // set next 2 bytes to rid
    pos += sizeof(uint16_t); // move over 2 bytes
    *(uint8_t*)pos = operation; // set next bte to operation
    pos += sizeof(u_int8_t); // move over 1 byte
    strcpy( pos, argv[4] ); // copy string into rest of packet

    // send packet
    write(sockfd, buf, 5 + strlen(argv[4]));
	
	uint8_t *in = malloc(MAXDATASIZE);
    if ((numbytes = read(sockfd, in, MAXDATASIZE-1)) == -1) {
        perror("recv");
        exit(1);
    }

    // disemvowel operation
	if (operation == 170) {
		uint8_t *inp = in; //pointer to packet
		uint16_t tml, rid; // set up variables
        //unpack (h,h,s) packet
		memcpy(&tml, inp, sizeof(uint16_t)); //copy first two bytes into tml
		inp += sizeof(uint16_t); //move pointer 2 bytes over
		memcpy(&rid, inp, sizeof(uint16_t)); //copy next two bytes into rid
		inp += sizeof(uint16_t); //move pointer 2 bytes over 
		char *inbuf = malloc(tml - 4);  //allocate space for string
		strcpy(inbuf, inp); //copy rest of packet into string
		inbuf[tml - 4] = '\0'; // set null terminator
        //print out some info
		printf("client: received tml is '%hu'\n",tml);
		printf("client: received rid is '%hu'\n",rid);
		printf("client: received disemvoweled message '%s'\n",inbuf);
	} 
    // vowel count operation
	else { 
		uint8_t *inp = in; // pointer to packet
		uint16_t tml, rid, vowels; //set up variables
        // unpack (h,h,h) uint16_t
		memcpy(&tml, inp, sizeof(u_short)); // copy first 2 bytes into tml
		inp += sizeof(uint16_t); // move pointer 2 bytes over
		memcpy(&rid, inp, sizeof(uint16_t)); // copy next 2 bytes into rid
		inp += sizeof(uint16_t); // move pointer 2 bytes over
		memcpy(&vowels, inp, sizeof(uint16_t)); // copy last two bytes into vowels
		printf("client: received tml is '%hu'\n",tml);
		printf("client: received rid is '%hu'\n",rid);
		printf("client: received vowel length '%hu'\n",vowels);
	}
	
    freeaddrinfo(servinfo); // all done with this structure
    close(sockfd);
    return 0;
}

