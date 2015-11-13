
/*
	This function
should transfer the first bytesToTransfer bytes of filename to the receiver at
hostname:hostUDPport correctly and efficiently
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
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "log.h"
#include "tcp.h"



#define PACKAGE_SIZE 7000
#define FAIL -1
#define OK 1


typedef struct {
	time_t      tv_sec;     /* seconds */
               suseconds_t tv_usec;    /* microseconds */
}timeval;

struct timeval end;
struct timeval start;

long estimated_RTT = 100000; 
long sample_RTT = 0;
//long DevRTT = 20000;

struct timeval t_send;
struct timeval t_receive;

int t_limit = 200000;
int n_time = 100000;

int dup_ack = 0;
int dup_flag = 0;
int threshold = 32;
int w_size = 1;
int cur_seq = 1;
int last_ack = -1;
int* ack_r;
int ignore_dup = -1;
log_t data_to_send;
Node *a;
unsigned long long int total = 0;

struct addrinfo *servinfo;

int socket_init(char* hostname, unsigned short int hostUDPport,struct addrinfo ** dest){
/* return a socket*/
	int sockfd;
	struct addrinfo hints, *p;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	char PORT[128];
	sprintf(PORT, "%d", hostUDPport);	


	if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
		return FAIL; // connection failure
	}
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;
		}
		break;
	}
	if (p == NULL) {
		return FAIL;//fail to bind
	}
	*dest = p;
	//freeaddrinfo(servinfo);
	return sockfd;
}

//add header later
int send_next_package(int socket, struct addrinfo * dest, size_t num_send, log_t* l,int first, int cur_index){//
	
	//printf("In_sender %d %d %d\n", num_send, first, cur_index);
	int numbytes;
	char* buffer;
	int a_length;
	char * current = malloc(PACKAGE_SIZE+sizeof(tcp_t));
	//char current[PACKAGE_SIZE + sizeof(tcp_t)];
	cur_index += first;
	int i;
	
	a = l -> head;
	
	for(i = 0; (i < first) && a != NULL; i++){
		a = a -> next;
	}

	for(i = first; (i < (first + num_send))&& (cur_index <= total) && a != NULL; i++, a = a->next, cur_index++){
		buffer = a -> data;
		a_length = a -> length;
		//buffer = log_search_index(l , i);
		if(buffer == NULL){
			printf("nope\n");
			return FAIL;
		}

		memset(current,0,PACKAGE_SIZE+sizeof(tcp_t));

		tcp_t header;

		memset(&header, 0, sizeof(tcp_t));

		//printf("-----reach--%d\n",cur_index);
		header.seq = cur_index;
		header.total = total;
		
		memcpy(current, &header, sizeof(tcp_t));

		//strncpy(current + sizeof(tcp_t), buffer, a_length);

		memcpy(current + sizeof(tcp_t),buffer,a_length);	
	
		//printf("%d\n",a_length);
		//strcat(current+ sizeof(tcp_t),"");
		
		//current[strlen(buffer) + sizeof(tcp_t)] = '\0';
		
		gettimeofday(&t_send, 0);
		if ((numbytes = sendto(socket, current, a_length + sizeof(tcp_t), 0, dest->ai_addr, dest->ai_addrlen)) <= 0) {//SEND first
			printf("It's a lose.\n");
			return FAIL; //fail to send data
		}
		//free(current);
		//printf("send: %d\n",numbytes);
	}
//strlen(buffer)+sizeof(tcp_t)+1
	free(current);
	return OK; //ok
}


unsigned long long int min(unsigned long long int a, unsigned long long int b){
	if(a <= b)
		return a;
	return b;
}
int Convert_File_To_List(log_t* Link_list,char * filename, unsigned long long int bytesToTransfer){
	//FILE* NOW;
	FILE* article;
	unsigned long long int num_byte = 0;
	unsigned long long int total_byte = 0;
	char Chunk_data[PACKAGE_SIZE];
	article = fopen (filename, "rb");
	//NOW = fopen("NOW","wb");
	unsigned long long int s_length = 0;	
	/*
	if(bytesToTransfer < sizeof(Chunk_data)){
		s_length = bytesToTransfer;
	}
	else{
		s_length = sizeof(Chunk_data);
	}
	*/
	s_length = min(bytesToTransfer, sizeof(Chunk_data));
	//printf("in_f\n");
	unsigned long long byte = bytesToTransfer;

	if (article){
		memset(Chunk_data, 0, sizeof Chunk_data);
    	while (((num_byte = fread(Chunk_data, sizeof(char), s_length, article)) > 0) && (total_byte < byte)){
			total_byte += num_byte;
			//Chunk_data[num_byte] = '\0';/////////
			log_push(Link_list, Chunk_data, num_byte);
			//fwrite(Chunk_data, sizeof(char), strlen(buffer), out);
			
			/*
			if(bytesToTransfer < sizeof(Chunk_data)){
				s_length = bytesToTransfer;
			}
			else{	
				s_length = sizeof(Chunk_data);
			}
			*/
			bytesToTransfer = bytesToTransfer - num_byte;
			s_length = min(bytesToTransfer, sizeof(Chunk_data));





			total++;
			memset(Chunk_data, 0, sizeof Chunk_data);
		}
	}
	else{
		printf("file not exist\n");
		return FAIL;
	}
	fclose(article);
	return OK;
}

void set_timeout(int sockfd, int timeout)
{
	struct timeval tv;
	tv.tv_sec = timeout/1000000;
	tv.tv_usec = timeout%1000000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt:");
	}
}

/*
int get_serial_number(char *ack){
	if (ack == NULL)
		return -1;
	return atoi(ack);
}
*/
int slow_start(int socket_fd,struct addrinfo * dest, log_t* data){

	/* set a window size of 1. retrive the exponetial-linear limit. 
		use a while loop, only break if special cases.
		in each loop. I send the window sized package to the receiver. Then check receive side whether received last one or not. if timeout and have not
		receive. resend. If receive seq ack. increase window size in some way. Continue send imediatly. If receive 3 times same ack of wrong ack. 
		go back to fast recovery. receive nothing at all. go slow.
	*/
	if(dup_flag == 1){//I don't set w_size to 1;
		dup_flag = 0;
	}else{
		w_size = 1;
	}
	
	
	int seq_n = 0;
	char ack_info[256];
	int next_index = 0;
	int numbytes;
	
	int rv = send_next_package(socket_fd, dest, w_size, data, 0, cur_seq);
	set_timeout(socket_fd, t_limit);
	if(rv < 0){
		printf("wo dao he bei sheng la\n");
		return -1;
	}
	while(1){
		//printf("slow_w_size: %d\n",w_size);
		numbytes = recvfrom(socket_fd, ack_info, sizeof(ack_info) , 0, NULL, NULL);
		if(numbytes < 0 && (errno == EAGAIN )){
			//slow restart;
			//if(seq_n >= total)
			//	return -1;
			//else
			//threshold = w_size%2 + w_size/2;
			estimated_RTT *= 2;
			printf("timeout\n");
				return 1;
		}else{
			gettimeofday(&t_receive, 0);
			sample_RTT = (t_send.tv_sec-t_receive.tv_sec)*1000000 + t_receive.tv_usec - t_send.tv_usec;
			estimated_RTT =( sample_RTT/8 + 7*estimated_RTT/8);
			
			//DevRTT = (3*DevRTT/4 + fabs((double)(sample_RTT-estimated_RTT))/4) ;
		}

		tcp_t * t = (tcp_t *)ack_info;
		seq_n = t->ack;		
		//printf("ack: %d\n",seq_n);
		if(seq_n > 0 && (seq_n <= total)){
			ack_r[seq_n - 1]++;
		}
		//printf("total: %d\n",total);
	//	if(seq_n >= total){
	//		printf("should be out but back in\n");
	//		return -1;
	//	}	
		//dup ack
		if((seq_n < 0)||(seq_n == ignore_dup)){
			continue;
		}else if(seq_n == last_ack){
			dup_ack++;
		}else if(seq_n >= cur_seq){// && seq_n < (cur_seq + w_size)) {//if dup_ack in the window
			last_ack = seq_n;
			dup_ack = 1;
		}

		if(dup_ack >= 3){ //reset current and go into fast recovery.
			cur_seq = seq_n + 1;
			ignore_dup = seq_n;/////
			//threshold = w_size%2 + w_size/2;
			w_size = w_size%2 + w_size/2;
			//threshold = w_size;
			printf("dup!\n");
			rv = send_next_package(socket_fd, dest, w_size, data, 0, cur_seq);
			continue;
			//return 2;
		}
		//deal with acks before window
		int t_size = w_size;
		if((seq_n < cur_seq) && (ack_r[seq_n-1] <= 1)){///// &&('seq_n havent seen before')
			t_size += 1;
		}//ack in the window
		else if( seq_n >= cur_seq && seq_n < (cur_seq + w_size)){
			next_index = cur_seq + w_size - seq_n - 1; //index in log, start from 0
			log_remove_head(data,seq_n-cur_seq);
			//printf("the cur_seq after remove: %d\n",cur_seq);
			cur_seq = seq_n + 1;
			t_size += 1;
		}
		else if (seq_n >= (cur_seq + w_size)){
			next_index = 0; //seq_n + w_size - cur_seq;
			log_remove_head(data,seq_n - cur_seq);
			//printf("the cur_seq after remove:%d\n",cur_seq);
			cur_seq = seq_n + 1;	
		}
		
		w_size = t_size;
		/*
		if(w_size == t_size){
			continue;
		}
		else{
			w_size = t_size;///////gai
		}
		*/
		
		if(w_size < threshold){
			rv = send_next_package(socket_fd, dest, w_size - next_index , data, next_index, cur_seq);//only send the new item
			set_timeout(socket_fd, n_time);
			if( (rv < 0) && (seq_n >= total)){
				printf("time to end\n");
				return -1;
			}
		}else if((cur_seq + w_size) <= total){//add
			printf("avoidance\n");
			return 2;
		}
	}
	return 0;
}




















/*
			w_size = w_size/2; 
			rv = send_next_package(socket_fd, dest, w_size - next_index , data, next_index, cur_seq);//only send the new item
			set_timeout(socket_fd, n_time);
			if( (rv < 0) && (seq_n >= total)){
				printf("time to end\n");
				return -1;
			}
			*/



void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer)
{

	//printf("%s %hu %s %llu\n",hostname ,hostUDPport,filename,bytesToTransfer);
	/*
		make a udp socket. reach the host and the destination port. waiting for receive ack. if ok, send part 2. otherwise nack send last part again.
	*/
	
	int socket_fd;
	//data
	struct addrinfo * dest;
	
	log_init(&data_to_send);
	
	
	if( (socket_fd = socket_init(hostname, hostUDPport, &dest)) < 0){
		return;
	}
	
	if( (Convert_File_To_List(&data_to_send , filename, bytesToTransfer)) < 0){
		return;
	}

	a = data_to_send.head;///////
	int rv = 1;
	//rv = slow_start(socket_fd, dest, &data_to_send);
	ack_r = calloc(total,sizeof(int));///
	
	while(data_to_send.head != NULL){
		//printf("timeout or dup or other?\n");
		switch(rv){
			case -1:{
				free(ack_r);
				freeaddrinfo(servinfo);
				return;
			}
			case 1:{
				rv = slow_start(socket_fd, dest, &data_to_send);
				break;
			}				
				//rv = slow_start(socket_fd, dest, &data_to_send);							
			//case 2:
			//	rv = slow_start(socket_fd, dest, &data_to_send);
			//	break;
			case 2:{
				//rv = slow_start(socket_fd, dest, &data_to_send);
				rv = congestion_avoidance(socket_fd, dest, &data_to_send);
				break;
			}
			//default:
			//	break;//rv = slow_start(socket_fd, dest, &data_to_send);
			
		}	
	}
	free(ack_r);
	freeaddrinfo(servinfo);
	/*
		after part 1, add the function of trafic control which includes increase window size and decrease window size. 
	*/
}


int main(int argc, char ** argv){
	if(argc <= 4)
		return 0;
		

	gettimeofday(&start, 0);
	reliablyTransfer(argv[1],(unsigned short int)atoi(argv[2]), argv[3],(unsigned long long int)atoi(argv[4]));
	gettimeofday(&end, 0);
	long elapsed = (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;
	printf("TIME:%ld\n",elapsed);
}























int congestion_avoidance(int socket_fd,struct addrinfo * dest, log_t* data){

	/* set a window size of 1. retrive the exponetial-linear limit. 
		use a while loop, only break if special cases.
		in each loop. I send the window sized package to the receiver. Then check receive side whether received last one or not. if timeout and have not
		receive. resend. If receive seq ack. increase window size in some way. Continue send imediatly. If receive 3 times same ack of wrong ack. 
		go back to fast recovery. receive nothing at all. go slow.
	*/
	double ack_count = 0;
	
	int seq_n = 0;
	char ack_info[256];
	int next_index = 0;
	int numbytes;
	int rv;
	//int rv = send_next_package(socket_fd, dest, w_size, data, 0, cur_seq);
	//set_timeout(socket_fd, t_limit);
	//if(rv < 0){
	//	printf("wo dao he bei sheng la\n");
	//	return -1;
	//}
	while(1){
		//printf("f_w_size: %d\n",w_size);
		numbytes = recvfrom(socket_fd, ack_info, sizeof(ack_info) , 0, NULL, NULL);
		if(numbytes < 0 && (errno == EAGAIN )){
			//threshold = w_size%2 + w_size/2;
			estimated_RTT *= 2;
			printf("timeout\n");
				return 1;
		}else{
			gettimeofday(&t_receive, 0);
			sample_RTT = (t_send.tv_sec-t_receive.tv_sec)*1000000 + t_receive.tv_usec - t_send.tv_usec;
			estimated_RTT =( sample_RTT/8 + 7*estimated_RTT/8);
		}

		tcp_t * t = (tcp_t *)ack_info;
		seq_n = t->ack;		
		//printf("ack: %d\n",seq_n);
		if(seq_n > 0 && (seq_n <= total)){
			ack_r[seq_n - 1]++;
		}
		//dup ack
		if((seq_n < 0)||(seq_n == ignore_dup)){
			continue;
		}else if(seq_n == last_ack){
			dup_ack++;
		}else if(seq_n >= cur_seq){// && seq_n < (cur_seq + w_size)) {//if dup_ack in the window
			last_ack = seq_n;
			dup_ack = 1;
		}

		if(dup_ack >= 3){ //reset current and go into fast recovery.
			cur_seq = seq_n + 1;
			ignore_dup = seq_n;/////
			//threshold = w_size%2 + w_size/2;
			w_size = w_size%2 + w_size/2;
			//threshold = w_size;
			printf("dup!\n");
			rv = send_next_package(socket_fd, dest, w_size, data, 0, cur_seq);
			continue;
		}
		//deal with acks before window
		int t_size = w_size;
		
		ack_count += 1/(double)w_size;
		if(ack_count >= 1){
			t_size += 1;
			ack_count = ack_count - 1;
		}
		
		if((seq_n < cur_seq) && (ack_r[seq_n-1] <= 1)){///// &&('seq_n havent seen before')
			t_size += 1;
		}//ack in the window
		else if( seq_n >= cur_seq && seq_n < (cur_seq + w_size)){
			next_index = cur_seq + w_size - seq_n - 1; //index in log, start from 0
			log_remove_head(data,seq_n-cur_seq);
			//printf("the cur_seq after remove: %d\n",cur_seq);
			cur_seq = seq_n + 1;
			t_size += 1;
		}
		else if (seq_n >= (cur_seq + w_size)){
			next_index = 0; //seq_n + w_size - cur_seq;
			log_remove_head(data,seq_n - cur_seq);
			//printf("the cur_seq after remove:%d\n",cur_seq);
			cur_seq = seq_n + 1;	
		}
		w_size = t_size;
		rv = send_next_package(socket_fd, dest, w_size - next_index , data, next_index, cur_seq);//only send the new item
		set_timeout(socket_fd, n_time);
		if( (rv < 0) && (seq_n >= total)){
			printf("time to end\n");
			return -1;
		}
	}
	return 0;
}









///////////////////////
/* set a window size of 1. retrive the exponetial-linear limit. 
		use a while loop, only break if special cases.
		in each loop. I send the window sized package to the receiver. Then check receive side whether received last one or not. if timeout and have not
		receive. resend. If receive seq ack. increase window size in some way. Continue send imediatly. If receive 3 times same ack of wrong ack. 
		go back to fast recovery. receive nothing at all. go slow.
	*/

/*
int congestion_avoidance(int socket_fd, struct addrinfo * dest, log_t* data){
	//don't change the w_size;
	double ack_count = 0;
	
	int dup_ack = 0;
	
	int seq_n = 0;
	char ack_info[256];
	int next_index = 0;
	int numbytes;
	int t_limit = 1000000;
	int n_time = 100000;
	int rv = send_next_package(socket_fd, dest, 1, data, w_size - 1, cur_seq);//should be correct
	set_timeout(socket_fd, t_limit);
	if(rv < 0)
		return 0;
	while(1){
		//printf("cog_w_size: %d\n",w_size);
		numbytes = recvfrom(socket_fd, ack_info, sizeof(ack_info) , 0, NULL, NULL);
		if(numbytes < 0 && (errno == EAGAIN )){
			//slow restart;
			printf("return 1\n");
			return 1;
		}
		tcp_t * t = (tcp_t *)ack_info;
		seq_n = t->ack;
		printf("c_ack: %d\n",seq_n);
		if(seq_n > 0 && seq_n < total){
			ack_r[seq_n - 1]++;
		}
		
		//dup ack
		if((seq_n < 0)||(seq_n == ignore_dup)){
			continue;
		}else if(seq_n == last_ack){
			dup_ack++;
		}else if(seq_n >= cur_seq && seq_n < (cur_seq + w_size)) {//if dup_ack in the window
			last_ack = seq_n;
			dup_ack = 1;
		}
		if(dup_ack >= 3){ //reset current and go into fast recovery.
			cur_seq = seq_n + 1;
			ignore_dup = seq_n;/////
			//w_size = w_size%2 + w_size/2;
			printf("dup!\n");
			//dup_flag = 1;
			//rv = send_next_package(socket_fd, dest, w_size, data, 0, cur_seq);
			//continue;
			return 1;//////////////////////////////////////////care
		}
		//deal with acks before window
		int t_size = w_size;
		if((seq_n < cur_seq) && (ack_r[seq_n-1] <= 1)){///// &&('seq_n havent seen before')
			//t_size += 1;
			ack_count += 1/(double)w_size;
			if(ack_count >= 1){
				t_size += 1;
				ack_count = ack_count - 1;
			}
		}//ack in the window
		else if( seq_n >= cur_seq && seq_n < (cur_seq + w_size)){
			next_index = cur_seq + w_size - seq_n - 1; //index in log, start from 0
			log_remove_head(data,seq_n-cur_seq);
			cur_seq = seq_n + 1;
			//t_size += 1;
			ack_count += 1/(double)w_size;
			if(ack_count > 1){
				t_size += 1;
				ack_count = ack_count - 1;
			}
		}
		else if (seq_n >= (cur_seq + w_size)){
			next_index = 0; //seq_n + w_size - cur_seq;
			log_remove_head(data,seq_n - cur_seq);
			cur_seq = seq_n + 1;	
		}
		
		
		w_size = t_size;
		rv = send_next_package(socket_fd, dest, w_size - next_index , data, next_index, cur_seq);//only send the new item
		set_timeout(socket_fd, n_time);
		if( (rv < 0) && (seq_n >= total)){
			printf("time to end\n");
			return -1;
		}
	}
	return 0;
}
*/


