/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "10014"    // the port users will be connecting to
#define MAXBUFLEN 100

struct __attribute__((packed)) packet{
    uint16_t tml;
    uint16_t requestid;
    uint8_t operation;
    char message[0];
};

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    printf("%s\n", argv[2]);
    // Setup test packet
    struct packet *test = malloc(sizeof(struct packet));
    test->tml = 5 + strlen(argv[2]) + 1;
    test->requestid = 1;
    test->operation = 0x55;
    test->message = strdup(argv[2]); //Copy argv[2] into struct

    printf("(%d|%d|%d|%s)\n", test->tml, test->requestid, test->operation, test->message); //Print test packet
    printf("Total packet size: %d\nLength of string: %d\n", sizeof(*test), strlen(test->message)); //Print size and length

    //printf("Message :%s %d\n", msgbuf, strlen(msgbuf));
    //Send each segment one by one
    // if ((numbytes = sendto(sockfd, &test.tml, sizeof(test.tml), 0, p->ai_addr, p->ai_addrlen)) == -1) {
    //     perror("talker: sendto");
    //     exit(1);
    // }
    // if ((numbytes = sendto(sockfd, &test.requestid, sizeof(test.requestid), 0, p->ai_addr, p->ai_addrlen)) == -1) {
    //     perror("talker: sendto");
    //     exit(1);
    // }
    // if ((numbytes = sendto(sockfd, &test.operation, sizeof(test.operation), 0, p->ai_addr, p->ai_addrlen)) == -1) {
    //     perror("talker: sendto");
    //     exit(1);
    // }
    // if ((numbytes = sendto(sockfd, test.message, strlen(test.message) + 1, 0, p->ai_addr, p->ai_addrlen)) == -1) {
    //     perror("talker: sendto");
    //     exit(1);
    // }

    if ((numbytes = sendto(sockfd, test, sizeof(*test) + strlen(test->message), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);

    close(sockfd);

    return 0;
}