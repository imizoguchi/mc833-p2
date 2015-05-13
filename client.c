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
#include <sys/time.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100000 // max number of bytes we can get at once 

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

    if (argc != 3) {
        fprintf(stderr,"usage: client hostname action\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
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

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    //printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    send(sockfd, "admin qwerty\n", 13, 0);

        struct timeval time_in, time_out;
        gettimeofday(&time_in,NULL);
        switch(atoi(argv[2])) {
            case 0:
                if (send(sockfd, "list title launchdate\n", 22, 0) == -1)
                    perror("send");
                break;
            case 1:
                if (send(sockfd, "genre Comedy\n", 13, 0) == -1)
                    perror("send");
                break;
            case 2:
                if (send(sockfd, "synopsis 10\n", 12, 0) == -1)
                    perror("send");
                break;
            case 3:
                if (send(sockfd, "get 11\n", 7, 0) == -1)
                    perror("send");
                break;
            case 4:
                if (send(sockfd, "list\n", 5, 0) == -1)
                    perror("send");
                break;
            case 5:
                if (send(sockfd, "reserve 12 1\n", 13, 0) == -1)
                    perror("send");
                break;
            case 6:
                if (send(sockfd, "copies 13\n", 10, 0) == -1)
                    perror("send");
                break;
        }
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        gettimeofday(&time_out,NULL);

        double time1, time2;

        // usec are microseconds!
        time1 = time_in.tv_sec + 0.000001*time_in.tv_usec;
        time2 = time_out.tv_sec + 0.000001*time_out.tv_usec;
        printf("%lf\n", time2-time1);

    send(sockfd, "exit\n", 5, 0);

    close(sockfd);

    return 0;
}
