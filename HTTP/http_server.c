/*
** http_server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char * argv[])
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

	if(argc != 2){
		fprintf(stderr,"usage: server port\n");
        exit(1);
	}
	char * port = argv[1];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	//printf("%s\n", port);
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	//printf("getaddrinfo\n");
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        //reuse port
        
		int optval = 1;  
  		setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
		
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
		//printf("bind\n");
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
	
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
		ssize_t len;
		char buf[1024];
		if(len = recv(new_fd, buf, 200, 0) == -1){
			perror("recv\n");
			
		};
		if(len > 0){
			buf[len] = '\0';
		}
		//printf("%d\n", len);
		//printf("%s\n", buf);
		char fname[1024];
		
		//printf("%s\n", fname);
		
		char headf[1024];
		FILE * fd = NULL;
		strcpy(headf, "HTTP/1.1 400 Bad Request\r\n\r\n");
		char * mess = NULL;
		if(strstr(buf, "GET") != NULL){
		
			sscanf(buf, "GET /%s HTTP/1.1\r\n\r\n", fname);
			// open file
			fd = fopen(fname, "rb");
			long size;
			//char mess[1024]; 
		
			if(fd == NULL){
				//send(); // file not fount
				strcpy(headf, "HTTP/1.1 404 Not Found\r\n\r\n");
				//exit(1);
			}
		
			else{
				fseek(fd, 0, SEEK_END);
				size = ftell(fd);
				fseek(fd, 0, SEEK_SET);
				// read in message
				mess = (char*)calloc(sizeof(char), size);
				printf("%d\n", size);
				fread(mess, 1, size, fd);
				//printf("%s\n", mess);
				strcpy(headf, "HTTP/1.1 200 OK\r\n\r\n");
				//fclose(fd);
			}
		
		}
		
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            if((send(new_fd, headf, strlen(headf), 0)) == -1){
            	perror("send head");
            }
            //printf("%d\n", strlen(mess));
            if(fd != NULL){
            	if((send(new_fd, mess, strlen(mess), 0)) == -1){
            		perror("send mess"); 
            		
            	}
            	/*
            	FILE * out = fopen("no", "wb");
            	fwrite(mess, sizeof(char), strlen(mess), out);
            	*/
            free(mess);
            	//printf("%s", mess);
            /*
            int r = fread(mess, sizeof(mess), 1, fd);
            printf("%d\n", r);
            while(!feof(fd)){
            	if((send(new_fd, mess, sizeof(mess), 0)) == -1){
            		perror("send mess");        
            	}
            	printf("%s", mess);
            	r = fread(mess, sizeof(mess), 1, fd);
            }
            */
            	fclose(fd);
            	printf("send file\n");
            }
            else{
            	printf("no file\n");
            }
            close(new_fd);
            exit(0);
        }
        
        /*
        if((send(new_fd, headf, sizeof(headf), 0)) == -1){
            	perror("send head");
            }
        char *line = NULL;
        size_t length = 0;
        while(!feof(fd)){
            getline(&line, &length, fd);
            printf("%s", line);
            send(new_fd, line, sizeof(line), 0);
          }
        send(new_fd, "\r\n\r\n", 5, 0);
        //printf("\r\n\r\n");
        free(line);
        fclose(fd);
        */
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
