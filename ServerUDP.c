/*
** listener.c -- a datagram sockets "server" demo
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

#define MYPORT "10014"    // the port users will be connecting to

#define MAXBUFLEN 100

struct packet{
    uint16_t tml;
    uint16_t requestid;
    uint8_t operation;
    char *message;
} __attribute__((packed));

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    struct packet message;
    while (1)
    {
        printf("listener: waiting to recvfrom...\n");

        int client_length = (int)sizeof(struct sockaddr_in);
        if ((numbytes = recvfrom(sockfd, &message, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &client_length)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        printf("listener: recieved\n");
        char *messagep = &message;
        uint16_t tml, requestid;
        uint8_t operation;
        memcpy(&tml,messagep,sizeof(uint16_t)); 
        messagep += sizeof(uint16_t); 
        memcpy(&requestid,messagep,sizeof(uint16_t)); 
        messagep += sizeof(uint16_t); 
        memcpy(&operation,messagep,sizeof(uint8_t));  
        messagep += sizeof(uint8_t); 
        int stringLength = tml - 5;
        printf("%d\n", stringLength);
        char buffer[1000];
        printf("%d|%d|%d\n", tml, requestid, operation);
        memcpy(buffer,messagep, stringLength);

        printf("(%d|%d|%d|%s)\n", message.tml, message.requestid, message.operation, buffer);

        printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
        printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("listener: packet contains \"%s\"\n", buf);

        if ((numbytes = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, client_length)) == -1) {
            perror("talker: sendto");
            exit(1);
        }
    }
    freeaddrinfo(servinfo);

    close(sockfd);

    return 0;
}