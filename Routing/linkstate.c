#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#define max_node 1024
#define infinity 2147483647
#define max(a,b) ((a)>(b)?(a):(b))


typedef struct{
	int cost;
	int prev;
	int flag;
}node;

typedef struct{
	int len;
	char * path;
	int next;
}s_node;

FILE * out;
int count;
int init[max_node][max_node];

node data[max_node][max_node];
s_node save[max_node][max_node];

void findroute();
void initSave();
void writemess(char* filename);
void initData(FILE * topo);
void Outputrout();


int main(int argc, char** argv){
	if(argc != 4){
		fprintf(stderr, "usage: %s topofile messagefile changefile\n\n", argv[0]);
		exit(1);
	}
	out = fopen("output.txt", "w");
	//printf("waht\n");
	int i, j;
	FILE * topo = fopen(argv[1], "r");
	//FILE * mess = fopen(argv[2], "r");
	FILE * change = fopen(argv[3], "r");
	
	initData(topo);                  // read in topo file and initiate data with store distance infor
	initSave();                      // initiate save which store path information
	fclose(topo);
	//printf("end init\n");
	findroute();                    // run dijkstra, update forwarding table and find path
	//printf("end route\n"); 
	Outputrout();                 // print forwarding table
	
	writemess(argv[2]);           // print message and path
	
	char temp[1024];
	while(fgets(temp, 1024, change) != NULL){
		if(strlen(temp) < 3)
			break;
		int a;
		int b;
		int dist;
		sscanf(temp, "%d %d %d", &a, &b, &dist);
									// read in each change and update init table
		init[a-1][b-1] = dist;
		init[b-1][a-1] = dist;
			
		count = max(count, a);
		count = max(count, b);
			//updata();
		
		initSave();                  // update save table
		
		findroute();                  // run dijkstra and check for updates
		//printf("check\n");
		Outputrout();                // print forwarding table
		//printf("aaa\n");
		writemess(argv[2]);           // print message and path
		//printf("--------\n");
	//	memset(&temp, 0, sizeof(temp));
	}
	
	fclose(out);
	fclose(change);
	return 0;
}

void initSave(){                      // this function initialize save table
	int i;
	int j;
	for(i = 0; i < max_node; i++){
		for(j = 0; j < max_node; j++){
			save[i][j].len = infinity;
			save[i][j].path = NULL;
			save[i][j].next = -1;
		}	
	}
}

void writemess(char* filename){           // this function write message and its path to output
	FILE * mess = fopen(filename, "r");
	int start;
	int end;
	char temp[1024];
	while(fgets(temp, 1024, mess) != NULL){
		
		int l = strlen(temp);
		if(l < 4)
			break;
		//printf("%s\n", temp);
		//temp[strlen(temp)-1] = '@';
		char a[1024];
		int kk = sscanf(temp, "%d %d %s", &start, &end, a);
		//printf("%d %d %d\n", kk, start, end);
		
		//temp[strlen(temp)] = '\0';
		
		//printf("%s\n", a);
		//printf("%c\n", a[0]);
		char * msg = strchr(temp, a[0]);
		if(save[start-1][end-1].len == 0){
			char buff[2048];
			sprintf(buff, "from %d to %d hops %d message %s", start, end, start, msg);
			fwrite(buff, sizeof(char), strlen(buff), out);
		}
		else if(save[start-1][end-1].len < infinity){
			char buff[2048];
			sprintf(buff, "from %d to %d hops %smessage %s", start, end, save[start-1][end-1].path, msg);
			fwrite(buff, sizeof(char), strlen(buff), out);
		}
		else if(save[start-1][end-1].len == infinity){
			char buff[2048];
			sprintf(buff, "from %d to %d hops unreachable message %s", start, end, msg);
			fwrite(buff, sizeof(char), strlen(buff), out);
		}
	}
	fclose(mess);
	

}


void findroute(){                    // this function basicly runs dijkstra's algorithm
	int s;
	int t;
	for(s = 0; s < count; s++){
		for(t = 0; t<count; t++){
			node state[count];
			int i;
			for(i = 0; i < count; i++){
				state[i].prev = -1;
				state[i].cost = infinity;
				state[i].flag = 0;
			}
			state[t].cost = 0;
			state[t].flag = 1;
			int k = t;
			int c = 0;
			do{
				if(s == t)
					continue;
				for(i = 0; i < count; i++){
					if(init[k][i]>=0 && state[i].flag == 0){
						if(state[k].cost + init[k][i] < state[i].cost){
							state[i].prev = k;
							state[i].cost = state[k].cost+init[k][i];
						}
						else if(state[k].cost + init[k][i] == state[i].cost){
							if(k < state[i].prev){
								state[i].prev = k;
							}
						}
					}
				}
				
				k = -1;
				int min = infinity;
				for(i = 0; i < count; i++){
					if(state[i].flag == 0 && state[i].cost < min){
						min = state[i].cost;
						k = i;
					}
				}
				if(k == -1){
					//printf("kkk\n");
					c = 1;
					continue;
				}
				state[k].flag = 1;
			}while(k!=s);
			if(c == 1){
				save[s][t].next = -1;
				save[s][t].len = infinity;
			}
			else{
			if(s == t){
				save[s][t].next = s;
				save[s][t].len = 0;
			}
			else{
				save[s][t].next = state[s].prev;
				save[s][t].len = state[s].cost;
			}
			//save[s][t].path = " ";
			int m = s;
			char * temp = malloc(1024);
			//temp  = " ";
			//strcat(save[s][t].path, )
		//		int a = state[s].prev;
		//	printf("what!\n");
		//	printf("%d %d\n", s, t);
			while(m != t){
				char ss[100];
				sprintf(ss, "%d ", m+1);
				//printf("%s\n", ss);
				//printf("%d\n", data[cur][end-1].prev+1);
				//printf("aaa\n");
				strcat(temp, ss);
				//printf("bbb\n");
				m = state[m].prev;
			}
			save[s][t].path = temp;
		//	printf("%s %d\n", save[s][t].path, save[s][t].len);
			}
		}
	}
}

void Outputrout(){                       // this function print forwarding table to output
	int i;
	int j;
	for(i = 0; i< count; i++){
		for(j = 0; j<count; j++){
			if(save[i][j].len >= 0 && save[i][j].len < infinity){
				char buff[100];
				sprintf(buff, "%d %d %d\n", j+1, save[i][j].next + 1, save[i][j].len);
				fwrite(buff, sizeof(char), strlen(buff), out);
			}
			else if(save[i][j].len == infinity){
				char buff[100];
				sprintf(buff, "%d -1 -1\n", j+1);
				fwrite(buff, sizeof(char), strlen(buff), out);
			}
			
		}
	}
}
/*
void writemess(argv[2]){



}
*/
void initData(FILE * topo){             // this function read in topo file 
	ssize_t size = 0;                    // and initialize distance information betweern routers
	char msg[1024];
	size_t read = 0;
	int i;
	int j;
	for(i = 0; i < max_node; i++){
		for(j = 0; j < max_node; j++){
			init[i][j] = -999;
			if(i == j){
				init[i][j] = 0;
				
			}
		}	
	}

	while(fgets( msg, 1024, topo) != NULL){
		int a;
		int b;
		int dist;
		sscanf(msg, "%d %d %d", &a, &b, &dist);
		//printf("%d\n", a);
		//printf("%d\n", b);
		//printf("%d\n", dist);
		
		init[a-1][b-1] = dist;
		init[b-1][a-1] = dist;
		
		count = max(count, a);
		count = max(count, b);
	}

}
