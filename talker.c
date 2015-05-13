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

#define SERVERPORT "4950"    // the port users will be connecting to
#define MAXDATASIZE 100000 // max number of bytes we can get at once 

int main(int argc, char *argv[])
{
    int sockfd;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname pre-defined-command-id\n");
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
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    sendto(sockfd, "admin qwerty\n", 13, 0, p->ai_addr, p->ai_addrlen);
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1, 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recv");
            exit(1);
    }

        struct timeval time_in, time_out;
        gettimeofday(&time_in,NULL);
        switch(atoi(argv[2])) {
            case 0:
                if (sendto(sockfd, "list title launchdate\n", 22, 0,p->ai_addr, p->ai_addrlen) == -1)
                    perror("send");
                break;
            case 1:
                if (sendto(sockfd, "genre Comedy\n", 13, 0,p->ai_addr, p->ai_addrlen) == -1)
                    perror("send");
                break;
            case 2:
                if (sendto(sockfd, "synopsis 10\n", 12, 0,p->ai_addr, p->ai_addrlen) == -1)
                    perror("send");
                break;
            case 3:
                if (sendto(sockfd, "get 11\n", 7, 0,p->ai_addr, p->ai_addrlen) == -1)
                    perror("send");
                break;
            case 4:
                if (sendto(sockfd, "list\n", 5, 0,p->ai_addr, p->ai_addrlen) == -1)
                    perror("send");
                break;
            case 5:
                if (sendto(sockfd, "reserve 12 1\n", 13, 0,p->ai_addr, p->ai_addrlen) == -1)
                    perror("send");
                break;
            case 6:
                if (sendto(sockfd, "copies 13\n", 10, 0,p->ai_addr, p->ai_addrlen) == -1)
                    perror("send");
                break;
        }
        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1, 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recv");
            exit(1);
        }

        gettimeofday(&time_out,NULL);

        double time1, time2;

        // usec are microseconds!
        time1 = time_in.tv_sec + 0.000001*time_in.tv_usec;
        time2 = time_out.tv_sec + 0.000001*time_out.tv_usec;
        printf("%s\n\n", buf);
        printf("%lf\n", time2-time1);

    sendto(sockfd, "exit\n", 5, 0,p->ai_addr, p->ai_addrlen);

    freeaddrinfo(servinfo);

    close(sockfd);

    return 0;
}