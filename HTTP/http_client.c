/*
** http_client.c -- a stream socket client demo
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

#include <arpa/inet.h>

#define PORT "80" // the port client will be connecting to 

#define MAXDATASIZE 1024 // max number of bytes we can get at once 
int hostname_ip(char*, char*);
void *get_in_addr(struct sockaddr *);

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int hostname_ip(char* hostname, char*ip){
	struct hostent *h;
	struct in_addr ** addr_list;
	int i;
	if((h = gethostbyname(hostname)) == NULL){
		herror("get hostname");
		return 1;
	}
	addr_list = (struct in_addr**)h->h_addr_list;
	for(i = 0; addr_list[i]!=NULL; i++){
		strcpy(ip, inet_ntoa(*addr_list[i]));
		return 0;
	}
	return 1;
}

int main(int argc, char *argv[])
{

	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	char mess[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char host[MAXDATASIZE];
	char page[MAXDATASIZE];
	char port[MAXDATASIZE];
	int a;
	int c = 0;

	if (argc != 2) {
        fprintf(stderr,"usage: ./http_client http://hostname:port/path/to/file\n");
        exit(1);
    }

	for(a=0; argv[1][a] != '\0'; a++){
		if(argv[1][a] == ':'){
			c ++;
		}
	}
	if(c==2){
		sscanf(argv[1], "http://%[^:]:%[^/]/%s", host, port, page);
	}
	else{
		sscanf(argv[1], "http://%[^/]/%s", host, page);
		//port = PORT;
		strcpy(port, PORT);
		//printf("check\n");
	}
	printf("%s %s %s \n", host, page, port);
	
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
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
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	
	
    //int sockfd, numbytes;  
    //struct addrinfo hints, *servinfo, *p;
    //int rv;

	//FILE * out = fopen("output", "w+");

    
    /*
	
	*/
	//int l = strlen(argv[1]);
	
	/*
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("client: socket");
		exit(1);
	};
	*/
	//printf("socket\n");
	
	//char ip[100];
	//get_ip(host, ip);
	/*
	hostname_ip(host, ip);

	struct sockaddr_in hints;
    bzero(&hints, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_port = htons(port);
	printf("%d\n", port);
	*/
    //memcpy(&hints.sin_addr, hent->h_addr_list[0], hent->h_length);
	// set the remote ip addrsscanf(argv[1], "http://%[^:]:%[^/]/%s", host, port, page);
   /*
    if(inet_pton(AF_INET, ip, &hints.sin_addr) <= 0){
    	perror("inet_pton");
    	exit(1);
    }
    printf("ip: %s\n", ip);
    */
    
   /*

    if((connect(sockfd, (struct sockaddr*)&hints, sizeof(hints))) < 0){
    	perror("client: connect");
    	exit(1);
    }
    
	printf("Connected: ");
	*/
	
/*
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
*/
	
	//char buffrec[1000];
	char buffsend[MAXDATASIZE];
	int num;

	// form GET request
	snprintf(buffsend, MAXDATASIZE, "GET /%s HTTP/1.1\r\n\r\n", page);
	//snprintf(buffsend, MAXDATASIZE, "GET / HTTP/1.1\r\nHost: www.baidu.com:80\r\n\r\n");
	// send request
	if((send(sockfd, buffsend, strlen(buffsend), 0)) < 0){
		perror("send\n");	
		exit(1);
	}
	//printf("%s\n", buffsend);
	// receive responce
	int flag = 0;
	FILE * out = fopen("output", "wb");
	
	char * buffrec = (char*)calloc(1, 5000);
	num = recv(sockfd, buffrec, 1000, 0);
	//printf("%s\n", buffrec);
	
	//printf("%d\n", num);
	while(num > 0){
		
		if( (flag == 0) && (strstr(buffrec, "\r\n\r\n") != NULL)){
			flag = 1;
			//printf("flag\n");
			char first[1000];
			char second[5000];
			
			char * p = strstr(buffrec, "\r\n\r\n");
			p+=4;
			fwrite(p, sizeof(char), strlen(p), out);
			size_t n = p - buffrec;
			strncpy(first, buffrec, n);
			first[n] = '\0';
			printf("%s", first);
			/*
			char *token;
			token = strtok(buffrec, "\r\n\r\n");
			printf("%s\r\n\r\n", token);
			*/
			/*
			token = strtok(NULL, "\r\n\r\n");
			printf("%s\n", token);
			size_t x = fwrite(token, sizeof(char), strlen(token), out);
			*/
			
			
		}
		else if(flag == 0){
			printf("%s\n", buffrec);
		}
		else{
			//printf("what\n");
			size_t x = fwrite(buffrec, sizeof(char), strlen(buffrec), out);
		}
		//size_t x = fwrite(buffrec, sizeof(char), strlen(buffrec), out);
		memset(buffrec, 0, 1000);
		num = recv(sockfd, buffrec, 1000, 0);
		//printf("%d\n", x);
	}
	
	/*
	num = recv(sockfd, buffrec, 5000, 0);
	printf("%s\n", buffrec);
	fwrite(buffrec, sizeof(char), strlen(buffrec), out);
	char buffer[500000];
	*/
	/*
	if(num = recv(sockfd, buffer, 500000, 0) != -1){
			buffer[num] = '\0';
			printf("%s", buffer);
			fwrite(buffer, 1, strlen(buffer), out);
	}
	*/
	/*
	while(1){
			//fwrite(buffrec, 1, num, out);
			// \r\n\r\n
			if(num = recv(sockfd, buffer, 500000, 0) != -1){
				buffer[num] = '\0';
				//printf("%s", buffrec);
				fwrite(buffer, 1, sizeof(buffer), out);
			}
			else{
				exit(1);	
			}
		}
	*/
    //printf("client: received '%s'\n",buf);
    printf("receive\n");
	fclose(out);
    close(sockfd);
    return 0;
}
