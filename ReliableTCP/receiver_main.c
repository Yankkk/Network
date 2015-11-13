#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "tcp.h"
//#define h_size sizeof(tcp_t)

//void reliablyReceive(unsigned short int myUDPport, char* destinationFile);
void reliablyReceive(char* myUDPport, char* destinationFile);
void write_file(char* buffer, int size);
void add_buffer(char*buff, int size, int seq);



FILE* out;
int payload = 7000;
int expect = 1;
int latest = 0;
int h_size = sizeof(tcp_t);

typedef struct list{
	int seq;
	char * data;
	int size;
	struct list *next;
    struct list *prev;
}list;

list * head = NULL;
list * tail = NULL;



int main(int argc, char** argv)
{
	unsigned short int udpPort;
	
	if(argc != 3)
	{
		fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
		exit(1);
	}
	
	udpPort = (unsigned short int)atoi(argv[1]);
	
	//reliablyReceive(udpPort, argv[2]);
	reliablyReceive(argv[1], argv[2]);
	
	return 0;
}

void reliablyReceive(char* myUDPport, char* destinationFile){
	/*
	int sockfd;
	struct sockaddr_in receiver, sender;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
		perror("open socket");
	
	memset((char*)&receiver, 0, sizeof(sockaddr_in));
	receiver.sin_family = AF_INET;
	receiver.sin_port = htons(myUDPport);
	receiver.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd, (struct sockaddr *)&receiver, sizeof(receiver))<0){
		perror("bind");
		exit(1);
	}
	*/
	
	int rv;
	int sockfd;
	
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t addr_len = sizeof(struct sockaddr);
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((rv =getaddrinfo(NULL, myUDPport, &hints, &servinfo))!= 0){
		 fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
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
        return ;
    }
	


    freeaddrinfo(servinfo);
    /// set connection
    
	
	out = fopen(destinationFile, "wb");
	//printf("%s\n", destinationFile);
	//char buffer[payload+h_size];
	char buffer[payload+sizeof(tcp_t)];
	//char* buffer = calloc(payload+h_size, 1);
	while(1){
		ssize_t recv = recvfrom(sockfd, buffer, payload+h_size, 0, (struct sockaddr *)&their_addr, &addr_len);
		//printf("%s\n", buffer);
		if(recv==-1){
			perror("recv");
			exit(1);
		}	
		/*
		char rec_s[h_size];
		memcpy(rec_s, buffer, h_size);
		int rec_seq = atoi(rec_s);
		*/
		//tcp_t header;
		//memset(&header, 0, sizeof(tcp_t));
		//header.seq = cur->seq;
		//header.total = total_seq;
		//memcpy(buffer, &header, sizeof(tcp_t));
		tcp_t * header = (tcp_t *)buffer;
		int rec_seq = header->seq;
		//printf("%d\n", rec_seq);
		//printf("%d\n", recv-sizeof(tcp_t));
		
		char * data = calloc(payload+1, 1);
		//strncpy(data, buffer+h_size, recv-sizeof(tcp_t));
		memcpy(data, buffer+h_size, recv-sizeof(tcp_t));
		//data[recv-sizeof(tcp_t)+1] = '\0';
		//if((recv-sizeof(tcp_t)) < 1024){
		//	printf("%d\n", recv-sizeof(tcp_t));
		//}

		//printf("%d\n", strlen(data));
		//char * data = buffer+h_size;
		if(rec_seq == expect){
			
			write_file(data, recv-h_size);			
			
			/*
			char seq[16];
			sprintf(seq, "%d", expect);
			sendto(sockfd, seq, strlen(seq), 0, (struct sockaddr*)&their_addr, addr_len);
			*/
			//printf("aaa%d\n", latest);
			tcp_t temp;
			memset(&temp, 0, sizeof(tcp_t));
			temp.ack = latest;
			sendto(sockfd, &temp, sizeof(tcp_t), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr));
		}
		else if(rec_seq > expect){             /// resend dup ack
												/// seq greater than expext
			/*
			char seq[16];
			sprintf(seq, "%d", latest);
			sendto(sockfd, seq, strlen(seq), 0, (struct sockaddr*)&their_addr, addr_len);
			*/
			tcp_t temp;
			memset(&temp, 0, sizeof(tcp_t));
			temp.ack = latest;
			sendto(sockfd, &temp, sizeof(tcp_t), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr));
			
			add_buffer(data, recv-h_size, rec_seq);          /// store data
		}
		else{                      /// seq less than expect
			/*
			char seq[16];
			sprintf(seq, "%d", latest);
			sendto(sockfd, seq, strlen(seq), 0, (struct sockaddr*)&their_addr, addr_len);
			*/
			tcp_t temp;
			memset(&temp, 0, sizeof(tcp_t));
			temp.ack = latest;
			sendto(sockfd, &temp, sizeof(tcp_t), 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr));
			
		}
		//printf("%d\n", latest);
		if(latest == header->total){
			/*
			list* ptr = head;
			while(ptr != tail){
				fwrite(ptr->data, sizeof(char), ptr->size, out);
				expect++;
				latest++;
				head = head->next;
				free(ptr->data);
				free(ptr);
				ptr = head;	
			}
		*/
			//printf("%s\n", data);
			free(tail);
			break;
		}
			
	}
	/*
	if(head == tail){
		free(tail);
	}
	*/
	//free(buffer);
	fclose(out);
	close(sockfd);
	return;
}

void add_buffer(char*buffer, int size, int seq){
	//char * buffer = malloc(size);
	//memcpy(buffer, buff, size);
	//free(buff);
	
	if(head == NULL && tail == NULL){
		head = calloc(sizeof(list), 1);
		tail = calloc(sizeof(list), 1);
		head->prev = NULL;
		head->seq = seq;
		head->size = size;
		head->data = buffer;
		head->next = tail;
		tail->prev = head;
		tail->next = NULL;
		tail->data = NULL;
		tail->size = -1;
	}
	else{
		list * temp = calloc(sizeof(list), 1);
		temp->seq = seq;
		temp->size = size;
		temp->data = buffer;
		temp->prev = tail->prev;
		temp->next = tail;
		tail->prev = temp;
	}

}


void write_file(char* buffer, int size){
	size_t t = fwrite(buffer, sizeof(char), size, out);
	//printf("%s\n", buffer);
	expect++;
	latest++;
	//printf("%ld\n", size);
	//printf("%ld\n", latest);
	//if(latest > 8500){
	//	printf("seq:%d\n", latest);
	//	printf("%s", buffer);
	//}
	//printf("aaa%d\n", latest);
	free(buffer);
	list * ptr = head;
	while(ptr != tail || ptr != NULL){
		if(ptr->seq == expect){
			fwrite(ptr->data, sizeof(char), ptr->size, out);
			expect++;
			latest++;
			//if(latest > 8500){
				//printf("seq:%d\n", latest);
				//printf("%s", ptr->data);
			//}
			head = head->next;
			free(ptr->data);
			free(ptr);
			ptr = head;	
		}
		else{
			break;
		}
	}


}

/*
void* send(void* ptr){
	while(1){
		char seq[15];
		sprintf(seq, "%d", ack);
		if(sendto(sockfd, , strlen(seq), p->ai_addr, p->ai_addrlen) == -1){
			perrpr("receiver: seq");
			exit(0);
		}
	}
}

int * reclist = (int*)calloc(sizeof(int), 1024);
int expect = 0;
char* rec_buffer = (char*)
int ack;

void* listen(void* ptr){
	int num_recv;
	char* buffer = (char*)calloc(sizeof(char), 1500);
	while(1){
		memset(buffer, 0, strlen(buffer));
		num_recv = recvfrom(sockfd, buffer, strlen(buffer), 0, NULL, NULL);
		
		if( (strstr(buffrec, "\r\n\r\n") != NULL)){
			char first[1000];
			char second[5000];
			
			char * p = strstr(buffrec, "\r\n\r\n");
			p+=4;
			
			size_t n = p - buffrec;
			strncpy(first, buffrec, n);
			first[n] = '\0';
			//printf("%s", first);
		    ack = atoi(first);
		    if(reclist[ack]==0){
		    	reclist[ack] = 1;
		    	if(expect == ack){
		    		fwrite(p, sizeof(char), strlen(p), out);
		    		expect ++;
		    		
		    	}
		    }	    
	}	
}
*/


