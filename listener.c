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
#include <stdarg.h>
#include <sys/time.h>

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 100000
#define MAXDATASIZE 100000

typedef struct movie {
    int id;
    char *title;
    char *launchDate;
    char *genre;
    char *synopsis;
    int copies;
} Movie;


int send_response(int sockfd, char *response, struct sockaddr_storage their_addr, socklen_t addr_len);
int handle_command(int sockfd, char *command, struct sockaddr_storage their_addr, socklen_t addr_len);
char *concat(int count, ...);

char *movie_to_string(Movie movie);
char *movie_projection(Movie movie, char **projection, int num_args);

Movie *movie;
int movie_count;
int is_root = 0;

void initialize_data() {

    movie = (Movie*) malloc(sizeof(Movie)*100);
    movie_count = 0;
    char buffer[10000];

    FILE *F = fopen("data.xls", "r");
    while(fgets(buffer, sizeof buffer, F)) {
        char *data = strtok(buffer, ";");
        movie[movie_count].id = strtol(data, NULL, 10);
        movie[movie_count].title = strdup(strtok(NULL, ";"));
        movie[movie_count].launchDate = strdup(strtok(NULL, ";"));
        movie[movie_count].genre = strdup(strtok(NULL, ";"));
        movie[movie_count].synopsis = strdup(strtok(NULL, ";"));
        movie[movie_count].copies = strtol(strtok(NULL, ";"), NULL, 10);

        movie_count++;
    }
}

void save_data() {
    FILE *F = fopen("data.xls", "w"); 
    for(int i = 0; i < movie_count; i++) {
        fprintf(F, "%d;%s;%s;%s;%s;%d;\n",
            movie[i].id,
            movie[i].title,
            movie[i].launchDate,
            movie[i].genre,
            movie[i].synopsis,
            movie[i].copies);
    }
        
    fclose(F);
}

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
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
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

    freeaddrinfo(servinfo);

    initialize_data();

    // printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
    while(1) {
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        // printf("listener: got packet from %s\n",
        //     inet_ntop(their_addr.ss_family,
        //         get_in_addr((struct sockaddr *)&their_addr),
        //         s, sizeof s));
        // printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        // printf("listener: packet contains \"%s\"\n", buf);


        char *command, *token, *saveptr_cmd, *saveptr_tok;
        int response;

        // Parse commands separated by linefeeds
        command = strtok_r(buf, "\n",&saveptr_cmd);
        while(command != NULL) {
            if(command[strlen(command)-1] == '\r')
                command[strlen(command)-1] = '\0';
            printf("server: received command '%s'\n",command);
            response = handle_command(sockfd, command,their_addr,addr_len);
            if(response == 0) {
                // remove user from current user sessions
                printf("disconnected client in address %s\n",inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));

            } else if(response == -1) {
                send_response(sockfd, "*** missing arguments\n\n",their_addr,addr_len);
            } else if(response == -2) {
                send_response(sockfd, "*** Access denied. Login as admin to perform this operation.\n\n",their_addr,addr_len);
            } else if(response == -404) {
                // invalid command
                send_response(sockfd, concat(3, "*** invalid command: ", command, "\n\n"),their_addr,addr_len);
            }
            command = strtok_r(NULL, "\n", &saveptr_cmd);
        }
    }

    close(sockfd);

    return 0;
}














int handle_command(int sockfd, char *command, struct sockaddr_storage their_addr, socklen_t addr_len) {
    char *token, *saveptr_tok, *info;
    char *args[10];

    struct timeval time_in, time_out;
    gettimeofday(&time_in,NULL);

    info = "";
                
    token = strtok_r(command, " ", &saveptr_tok);
    if(token != NULL) {

        printf("Received command: %s\n", token);

        // Exit
        if(strcmp(token, "exit") == 0) {
            return 0;

        /* Get all information of a specific movie
            _id:  id of the desired movie */
        } else if(strcmp(token, "admin") == 0) {
            token = strtok_r(NULL, " ", &saveptr_tok);
            if(token == NULL) return -1;

            if(strcmp(token, "qwerty") == 0) {
                is_root = 1;
                info = "Logged in as admin.\n\n";
            } else {
                info = "Wrong password.\n\n";
            }

        /* Get all information of a specific movie
            _id:  id of the desired movie */
        } else if(strcmp(token, "get") == 0) {
            token = strtok_r(NULL, " ", &saveptr_tok);
            if(token == NULL) return -1;
            int id = strtol(token, NULL, 10);

            for(int i = 0; i < movie_count; i++) {
                if(movie[i].id == id) {
                    info = movie_to_string(movie[i]);
                    break;
                }
            }

        /* Get the synopsis of a specific movie
            _id:  id of the desired movie's synopsis */
        } else if(strcmp(token, "synopsis") == 0) {
            token = strtok_r(NULL, " ", &saveptr_tok);
            int id = strtol(token, NULL, 10);
            if(token == NULL) return -1;

            for(int i = 0; i < movie_count; i++) {
                if(movie[i].id == id) {
                    info = movie[i].synopsis;
                    break;
                }
            }

        /* Reserve a number of copies of a specific movie.
            Gives an error when not possible to reserve (lack of copies)
            _id:            id of the desired movie to be reserved
            _num_copies:    number of copies to be reserved */
        } else if(strcmp(token, "reserve") == 0) {
            if(is_root) {
                token = strtok_r(NULL, " ", &saveptr_tok);
                if(token == NULL) return -1;

                int id = strtol(token, NULL, 10);

                token = strtok_r(NULL, " ", &saveptr_tok);
                if(token == NULL) return -1;

                int num = strtol(token, NULL, 10);

                for(int i = 0; i < movie_count; i++) {
                    if(movie[i].id == id) {
                        if(movie[i].copies >= num) {
                            movie[i].copies -= num;
                            info = concat(4, info, "Reserved ", token, " copies successfully.\n");

                        } else {
                            info = "Operation cancelled. Not enough copies.\n";
                        }
                        break;
                    }
                }
            } else return -2;

        /* return a number of copies of a specific movie. Can be
        used to add new copies of a movie to the store.
            _id:            id of the desired movie to be reserved
            _num_copies:    number of copies to be freed */
        } else if(strcmp(token, "return") == 0) {
            if(is_root) {
                token = strtok_r(NULL, " ", &saveptr_tok);
                if(token == NULL) return -1;
                int id = strtol(token, NULL, 10);
                token = strtok_r(NULL, " ", &saveptr_tok);
                if(token == NULL) return -1;
                int num = strtol(token, NULL, 10);

                for(int i = 0; i < movie_count; i++) {
                    if(movie[i].id == id) {
                        movie[i].copies += num;
                        info = concat(4, info, "Returned ", token, " copies successfully.\n");
                        break;
                    }
                }
            } else return -2;

        /* Get the number of copies of a specific movie.
            _id:  id of the desired movie's number of copies */
        } else if(strcmp(token, "copies") == 0) {
            token = strtok_r(NULL, " ", &saveptr_tok);
            int id = strtol(token, NULL, 10);

            for(int i = 0; i < movie_count; i++) {
                if(movie[i].id == id) {
                    char buffer[20];
                    sprintf(buffer, "%d", movie[i].copies);
                    info = concat(3, "There are ", buffer, " copies available.\n");
                    break;
                }
            }

        /* Get specific information of all movies.
            _id:    id of the desired movie's synopsis
            args*:  the projection to be returned */
        } else if(strcmp(token, "genre") == 0) {

            int count = 0;
            token = strtok_r(NULL, " ", &saveptr_tok);
            if(token == NULL) return -1;
            
            for(int i = 0; i < movie_count; i++) {
                if(strcmp(movie[i].genre,token) == 0) {
                    info = concat(2, info, movie_to_string(movie[i]));
                }
            }

        /* Get specific information of all movies.
            _id:    id of the desired movie's synopsis
            args*:  the projection to be returned */
        } else if(strcmp(token, "list") == 0) {

            int count = 0;
            token = strtok_r(NULL, " ", &saveptr_tok);

            if(token == NULL) {
                for(int i = 0; i < movie_count; i++)
                    info = concat(2, info, movie_to_string(movie[i]));
            } else {
                while(token != NULL) {
                    args[count] = strdup(token);
                    printf("%s ", args[count]);
                    count++;
                    token = strtok_r(NULL, " ", &saveptr_tok);
                }

                for(int i = 0; i < movie_count; i++) {
                    info = concat(2, info, movie_projection(movie[i], args, count));
                }
            }
        }  else {
            return -404;
        }
    }

    gettimeofday(&time_out,NULL);

    double time1, time2;

    // usec are microseconds!
    time1 = time_in.tv_sec + 0.000001*time_in.tv_usec;
    time2 = time_out.tv_sec + 0.000001*time_out.tv_usec;
    printf("execution time: %lf\n", time2-time1);
    send_response(sockfd,info,their_addr,addr_len);
    return 1;
}

char *movie_to_string(Movie movie) {
    char *movie_info;
    char buffer[20];
    
    sprintf(buffer, "Movie id: %d", movie.id);
    movie_info = strdup(buffer);
    movie_info = concat(3, movie_info, "\n\tTitle: ", movie.title);
    movie_info = concat(3, movie_info, "\n\tLaunched at: ", movie.launchDate);
    movie_info = concat(3, movie_info, "\n\tGenre: ", movie.genre);
    movie_info = concat(3, movie_info, "\n\tSynopsis: ", movie.synopsis);
    sprintf(buffer, "%d", movie.copies);
    movie_info = concat(3, movie_info, "\n\tAvailable copies: ", buffer);

    movie_info = concat(2, movie_info, "\n\n");
    return movie_info;
}

char *movie_projection(Movie movie, char **projection, int num_args) {
    char *movie_info;
    char buffer[20];

    sprintf(buffer, "Movie id: %d", movie.id);
    movie_info = strdup(buffer);

    for(int i = 0; i < num_args; i++) {
        if(strcmp(projection[i], "title") == 0) movie_info = concat(3, movie_info, "\n\tTitle: ", movie.title);
        if(strcmp(projection[i], "launchdate") == 0) movie_info = concat(3, movie_info, "\n\tLaunched at: ", movie.launchDate);
        if(strcmp(projection[i], "genre") == 0) movie_info = concat(3, movie_info, "\n\tGenre: ", movie.genre);
        if(strcmp(projection[i], "synopsis") == 0) movie_info = concat(3, movie_info, "\n\tSynopsis: ", movie.synopsis);
        if(strcmp(projection[i], "copies") == 0) {
            sprintf(buffer, "%d", movie.copies);
            movie_info = concat(3, movie_info, "\n\tAvailable copies: : ", buffer);
        }
    }
    movie_info = concat(2, movie_info, "\n\n");
    return movie_info;
}

int send_response(int sockfd, char *response, struct sockaddr_storage their_addr, socklen_t addr_len) {
    int numbytes;
    if ((numbytes = sendto(sockfd, response, strlen(response), 0,
                           (struct sockaddr *)&their_addr, addr_len)) == -1) {
      perror("listener: sendto");
      exit(1);
    }   
    return numbytes;
}

char* concat(int count, ...)
{
    va_list ap;
    int len = 1, i;

    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    char *result = (char*) calloc(sizeof(char),len);
    int pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(result+pos, s);
        pos += strlen(s);
    }
    va_end(ap);

    return result;
}