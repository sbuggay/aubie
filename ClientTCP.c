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

    //seed rng
    srand(time(NULL));

    if (argc != 5) { // check for correct args
        fprintf(stderr,"usage: client [hostname] [port] [operation] [message]\n");
        exit(1);
    }

    printf("ClientUDP.c group 4\n");

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
            perror("Client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Client: failed to connect\n");
        return 2;
    }
    // setup variables
	uint16_t requestid = rand() % 65535; // requestid can be whatever
	uint8_t operation = atoi(argv[3]); // set operation
    char *message = argv[4];
    uint16_t tml = 5 + strlen(argv[4]);

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("Client: connecting to %s\n", s);

    printf("sending tml %hu\n", tml);
    printf("sending requestid: %hu\n", requestid);
    printf("sending operation: %u\n", operation);
    printf("sending message: %s\n", message);

    // convert hose to network byte order
    tml = htons(tml);
    requestid = htons(requestid);

    // create packet
    uint8_t *buf = malloc(tml); // allocate space for packet
    uint8_t *pos = buf; //point pointer
    *(uint16_t*)pos = tml; //set first 2 bytes to total message length (5 + strlen(message))
    pos += sizeof(uint16_t); // move pointer 2 bytes over
    *(uint16_t*)pos = requestid; // set next 2 bytes to rid
    pos += sizeof(uint16_t); // move over 2 bytes
    *(uint8_t*)pos = operation; // set next bte to operation
    pos += sizeof(uint8_t); // move over 1 byte
    strcpy(pos, message); // copy string into rest of packet

    // Time is not very accurate in C :|...

    // send packet
    write(sockfd, buf, 5 + strlen(argv[4]));

    printf("Client: sent packet\n");
	
	uint8_t *in = malloc(MAXDATASIZE);
    if ((numbytes = read(sockfd, in, MAXDATASIZE-1)) == -1) {
        perror("recv");
        exit(1);
    }

    printf("Client: received packet\n");

    // print time elapsed

    // disemvowel operation
	if (operation == 170) {
        printf("Operation 170: Disemvowel\n");
		uint8_t *inp = in; //pointer to packet
		uint16_t tml, rid; // set up variables
        //unpack (h,h,s) packet
		memcpy(&tml, inp, sizeof(uint16_t)); //copy first two bytes into tml
		inp += sizeof(uint16_t); //move pointer 2 bytes over
		memcpy(&rid, inp, sizeof(uint16_t)); //copy next two bytes into rid
		inp += sizeof(uint16_t); //move pointer 2 bytes over 
        tml = ntohs(tml); //convert network to host
        rid = ntohs(rid);
		char *inbuf = malloc(tml - 4);  //allocate space for string
		strcpy(inbuf, inp); //copy rest of packet into string
        inbuf[tml - 4] = '\0';

        //print out some info
		printf("received tml: %hu\n",tml);
		printf("received requestid: %hu\n",rid);
		printf("received message: %s\n",inbuf);
	} 
    // vowel count operation
	if (operation == 85) {
        printf("Operation 85: Number of vowels\n"); 
		uint8_t *inp = in; // pointer to packet
		uint16_t tml, rid, vowels; //set up variables
        // unpack (h,h,h) uint16_t
		memcpy(&tml, inp, sizeof(uint16_t)); // copy first 2 bytes into tml
		inp += sizeof(uint16_t); // move pointer 2 bytes over
		memcpy(&rid, inp, sizeof(uint16_t)); // copy next 2 bytes into rid
		inp += sizeof(uint16_t); // move pointer 2 bytes over
		memcpy(&vowels, inp, sizeof(uint16_t)); // copy last two bytes into vowels

        tml = ntohs(tml); //convert network to host
        rid = ntohs(rid);
        vowels = ntohs(vowels);

		printf("received tml: %hu\n",tml);
		printf("received requestid: %hu\n",rid);
		printf("received vowel length: %hu\n",vowels);
	}
	
    freeaddrinfo(servinfo); // all done with this structure
    close(sockfd);
    return 0;
}

