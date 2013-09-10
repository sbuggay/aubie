// Group 4
// ServerUDP.c
// compile with gcc ServerUDP.c -o ServerUDP
// run with ./ServerUDP [port]

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

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int is_vowel(char c) { //check for vowels
    switch(c)
    {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
        case 'A':
        case 'E':
        case 'I':
        case 'O':
        case 'U':
        return 1;
        default:
        return 0;
    }
}

void removeChar(char *str, char c) { //removeChar function to remove characters from a string
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != c) dst++;
    }
    *dst = '\0';
}

int main(int argc, char *argv[])
{
    if(argc != 2) { //check for correct usage
        printf("Usage:./server port\n");
        exit(0);
    }

    char* port = argv[1]; //set the port

    //init a bunch of variables
    int sockfd, rv, numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN], s[INET6_ADDRSTRLEN];;
    socklen_t addr_len;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) { // loop through all the results and bind to the first we can
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: bind");
            continue;
        }
        break;
    }

    if (p == NULL) { //Check if bound
        fprintf(stderr, "Server: failed to bind socket\n");
        return 2;
    }

    int client_length = (int)sizeof(struct sockaddr_in); //set client length

    //infinite loop
    while (1)
    {
        printf("Server: waiting to recvfrom...\n");
        char *s = NULL;
        
        //struct request_packet *message = malloc(MAXBUFLEN - sizeof(struct request_packet) - 1);
        uint8_t *message = malloc(MAXBUFLEN - 1); //allocate space for the message

        //recieve a request
        if ((numbytes = recvfrom(sockfd, message, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &client_length)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        printf("Server: recieved\n");
        printf("Server: packet is %d bytes long\n", numbytes);

        //unpack packet
        uint8_t *messagep = message; //new char pointer at message address
        uint16_t tml, requestid; //set up vars
        uint8_t operation;
        memcpy(&tml,messagep,sizeof(uint16_t)); //copy first 2 bytes into tml 
        messagep += sizeof(uint16_t); //shift over 2 bytes
        memcpy(&requestid,messagep,sizeof(uint16_t)); //copy 2nd 2 bytes into requestid
        messagep += sizeof(uint16_t); //shift over 2 bytes
        memcpy(&operation,messagep,sizeof(uint8_t)); //copy next byte into operation
        messagep += sizeof(uint8_t); //shift over 1 byte
        int stringLength = tml - 5; //calculate length of string (tml - 5(2 + 2 + 1 for header))
        char *buffer = malloc(stringLength + 1); //allocate space including space for '\0'
        strcpy(buffer, messagep); // copy the rest of the messagep into buffer string
        buffer[stringLength] = '\0'; //set '\0'

        // Print some info on the packet
        printf("(%d|%d|%d|%s)\n", tml, requestid, operation, buffer);
        printf("tml: %d\n", tml);
        printf("requestid: %d\n", requestid);
        printf("operation: %d\n", operation);
        printf("string: %s\n", buffer);
        
        // Number of Vowels
        if(operation == 85) {
            printf("Operation 85: Number of vowels\n");
            printf("String: %s\n", buffer);
            uint16_t total = 0;
            char c;
            char *bufferp = buffer;
            while(c = *bufferp++) {
                if(is_vowel(c)) {
                    total++;
                }
            } 
            uint16_t out[3];
            out[0] = 6; //TML will always be 6 because message will always contain 3 shorts
            out[1] = requestid; //send back requestid
            out[2] = total; //set total
            printf("Total vowels: %d\n", total);
            printf("Server: Sending response to client\n");
            sendto(sockfd, out, 6, 0, (struct sockaddr *)&their_addr, client_length); //sends the out array
        }
        // Disemvowel
        else if(operation == 170) {
            printf("Operation 107: Disemvowel\n");
            printf("String in: %s\n", buffer);
            removeChar(buffer, 'a'); //remove all vowels
            removeChar(buffer, 'e');
            removeChar(buffer, 'i');
            removeChar(buffer, 'o');
            removeChar(buffer, 'u');
            removeChar(buffer, 'A');
            removeChar(buffer, 'E');
            removeChar(buffer, 'I');
            removeChar(buffer, 'O');
            removeChar(buffer, 'U');
            printf("String out: %s\n", buffer);

            //set up returning packet
            char *buf = malloc(4 + strlen(buffer)); //allocate space (4 + length of returning string)
            char *pos = buf; //set up pointer
            *(u_short*)pos = 4 + strlen(buffer); //fill in tml
            pos += sizeof(u_short); //shift
            *(u_short*)pos = requestid; //fill in requestid
            pos += sizeof(u_short); //shift
            strcpy( pos, buffer ); //fill in string
            //packet now looks like (h,h,s)

            printf("Server: Sending response to client\n");
            int bytes = sendto(sockfd, buf, 4 + strlen(buffer), 0, (struct sockaddr *)&their_addr, client_length); //send the packet to the client
            printf("Sent %d bytes\n", bytes);
        }
        // No op
        else {
            printf("Server: Unknown operation\n");
        }
        //free everything
        free(message);
        free(buffer);
        //null all the pointers
        message = NULL;
        messagep = NULL;
        buffer = NULL;
    }
    // Cleanup
    freeaddrinfo(servinfo);
    close(sockfd);
    return 0;
}