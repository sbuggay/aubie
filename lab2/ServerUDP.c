// Group 4
// ServerUDP.c
// compile with gcc ServerUDP.c -o ServerUDP
// run with ./ServerUDP [port]

//uncomment this line if you wish to print out debug info
#define debug

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


#define MAXBUFLEN 1000

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

// function for calculating checksum
uint8_t calc_checksum(uint8_t *p, int size);

//function for getting the ip address from the hostname
uint32_t hostname_to_ip(char * hostname , char* ip);

int main(int argc, char *argv[])
{
    if(argc != 2) { //check for correct usage
        printf("Usage:./ServerUDP [port]\n");
        exit(0);
    }

    printf("ServerUDP.c lab 2 group 4 (DNS Server)\n");

    char* port = argv[1]; //set the port
    
    //init a bunch of variables
    int sockfd, rv, numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    //char buf[MAXBUFLEN], s[INET6_ADDRSTRLEN];;
    // socklen_t addr_len;
    
    //set the hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    //get address info
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) { 
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("[SERVER] socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("[SERVER] bind");
            continue;
        }
        break;
    }

    if (p == NULL) { //Check if bound
        fprintf(stderr, "[SERVER] failed to bind socket\n");
        return 2;
    }

    int client_length = (int)sizeof(struct sockaddr_in); //set client length
    
    //infinite loop
    while (1)
    {
        printf("[SERVER] waiting to recieve...\n");
        
        uint8_t *message = malloc(MAXBUFLEN - 1); //allocate space for the message

        //recieve a request
        if ((numbytes = recvfrom(sockfd, message, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &client_length)) == -1) {
            perror("recvfrom");
            exit(1);
        }

#ifdef debug
        printf("[SERVER] recieved packet (%d bytes)\n", numbytes);
#endif

        //unpack packet
        uint8_t *messagep = message; //new char pointer at message address
        uint16_t tml; //set up vars
        uint8_t checksum, requestid, gid;
        memcpy(&tml, messagep, sizeof(uint16_t)); //copy first 2 bytes into tml 
        messagep += sizeof(uint16_t); //shift over 2 bytes
        memcpy(&checksum, messagep, sizeof(uint8_t)); //copy 1 byte into requestid
        messagep += sizeof(uint8_t); //shift over 1 bytes
        memcpy(&gid, messagep, sizeof(uint8_t)); //copy next byte into operation
        messagep += sizeof(uint8_t); //shift over 1 byte
        memcpy(&requestid, messagep, sizeof(uint8_t)); //copy next byte into operation
        messagep += sizeof(uint8_t); //shift over 1 byte

        //convert network to host byte order
        tml = ntohs(tml);


        //get string of hostnames out
        int stringLength = tml - 5; //calculate length of string (tml - 5(2 + 2 + 1 for header))
        char *buffer = malloc(stringLength + 1); //allocate space including space for '\0'
        strcpy(buffer, messagep); // copy the rest of the messagep into buffer string
        buffer[stringLength] = '\0'; //set null terminator

#ifdef debug
        // Print some debug info on the packet
        printf("[DEBUG] packet:[%d|%d|%d|%d|%s]\n", tml, checksum, gid, requestid, buffer);
        printf("[DEBUG] recieved tml: %d\n", tml);
        printf("[DEBUG] recieved checksum: %d\n", checksum);
        printf("[DEBUG] recieved gid: %d\n", gid);
        printf("[DEBUG] recieved reqestid: %d\n", requestid);
        printf("[DEBUG] recieved hostname_list: %s\n", buffer);
#endif



        //checksum error handling
        if (calc_checksum(message, tml) != 0) { // will turn out to be 0 because calc_checksum returns the ~ inverse of it and if the checksum comes out to be FF it will be 0

            printf("[SERVER] Checksum invalid, sending error message to client\n");
            
            //set up packet to send
            char *buf = malloc(5); //allocate space (4 + length of returning string)
            char *pos = buf; //set up pointer
            *(uint8_t*)pos = 0; //fill in checksum
            pos += sizeof(uint8_t); //shift
            *(uint8_t*)pos = gid; //fill in gid
            pos += sizeof(uint8_t); //shift
            *(uint8_t*)pos = requestid; //fill in requestid
            pos += sizeof(uint8_t); //shift
            *(uint16_t*)pos = htons(0x0000); //fill in requested two bytes
            

            //print out packet
            printf("[DEBUG] packet:[%d|%d|%d|%x]\n", checksum, gid, requestid, 0x0000);

            //send error message
            sendto(sockfd, buf, 5, 0, (struct sockaddr *)&their_addr, client_length); //sends the out array
            printf("[SERVER] sent checksum error message\n");

            //free the message buffer
            free(buf);
        }
        //length mismatch error handling
        else if (tml != numbytes) {
            printf("[SERVER] Message length invalid: sending error message to client\n");
            
            //set up packet to send
            char *buf = malloc(5); //allocate space (4 + length of returning string)
            char *pos = buf; //set up pointer
            *(uint8_t*)pos = checksum; //fill in checksum
            pos += sizeof(uint8_t); //shift
            *(uint8_t*)pos = 127; //fill in gid
            pos += sizeof(uint8_t); //shift
            *(uint8_t*)pos = 127; //fill in requestid
            pos += sizeof(uint8_t); //shift
            *(uint16_t*)pos = htons(0x0000); //fill in requested two bytes
            checksum = calc_checksum(buf, tml);
            pos = buf;
            pos += sizeof(uint16_t);
            *(uint8_t*)pos = checksum;

            //send error message
            sendto(sockfd, buf, 5, 0, (struct sockaddr *)&their_addr, client_length); //sends the out array
            printf("[SERVER] sent length mismatch error message\n");

            //free the message buffer
            free(buf);
        }
        //no errors, complete request
        else {
            printf("[SERVER] Checksum valid\n");
            uint16_t total_hostnames = 0;
            uint32_t ip_list[100];
            char *pch = strtok (buffer,"~");
            while (pch != NULL) {
                char ip[100];
                uint32_t iip = hostname_to_ip(pch, ip);
                printf("[DEBUG] %s resolved to %s (%u)\n", pch, ip, iip);
                ip_list[total_hostnames] = iip;
                pch = strtok(NULL, "~");
                total_hostnames++;
            }
            uint16_t tml = 5 + (4 * total_hostnames);
            
            int i;

            char *buf = malloc(tml); //allocate space (4 + length of returning string)
            char *pos = buf; //set up pointer
            *(uint16_t*)pos = htons(tml); //fill in tml
            pos += sizeof(uint16_t); //shift
            *(uint8_t*)pos = 0; //fill in checksum
            pos += sizeof(uint8_t); //shift
            *(uint8_t*)pos = gid; //fill in gid
            pos += sizeof(uint8_t); //shift
            *(uint8_t*)pos = requestid; //fill in requestid
            pos += sizeof(uint8_t); //shift
            for (i = 0; i < total_hostnames; i++) {
                *(uint32_t*)pos = htonl(ip_list[i]);
                pos += sizeof(uint32_t);
            }
            checksum = calc_checksum(buf, tml);
            pos = buf;
            pos += sizeof(uint16_t);
            *(uint8_t*)pos = checksum;

#ifdef debug
            //print out packet
            
            printf("[DEBUG] packet:[0x%X|0x%X|0x%X|0x%X", tml, checksum, gid, requestid);

                for(i = 0; i < total_hostnames; i++) {
                    printf("|0x%X", ip_list[i]);
                }
                printf("]\n");
                printf("[DEBUG] sending tml: %d\n", tml);
                printf("[DEBUG] sending checksum: %d\n", checksum);
                printf("[DEBUG] sending gid: %d\n", gid);
                printf("[DEBUG] sending requestid: %d\n", requestid);
#endif


            //send request message
            int bytes = sendto(sockfd, buf, tml, 0, (struct sockaddr *)&their_addr, client_length); //sends the out array
            printf("[SERVER] sent message back to client (%d bytes)\n", bytes);

            //free the message buffer
            free(buf);
        }    
    }
    // cleanup everything
    freeaddrinfo(servinfo);
    close(sockfd);
    return 0;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

uint8_t calc_checksum(uint8_t *p, int size) {
    //return 0;
    int i;
    uint16_t buffer = 0;
    for (i = 0; i < size; i++) {
        buffer += p[i]; // add the byte to the buffer
        uint8_t carry = buffer >> 8; // get the carry
        buffer += carry; // add back the carry
        buffer = buffer & 0xff; // get only the last 8 bits
        //printf("%u : %u\n", p[i], buffer);
    }
    return (uint8_t) ~buffer; // return the 1's compliment
    // This means that when the entire checksum is calculated, if it a correct one, it should return 0. (~0xFF = 0x00)
}

uint32_t hostname_to_ip(char *hostname, char *ip) {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL) {
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++) {
        strcpy(ip, inet_ntoa(*addr_list[i]) );
        return addr_list[i]->s_addr;
    }

    return 0;
}