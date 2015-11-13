#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define max_node 1024
#define max(a,b) ((a)>(b)?(a):(b))


typedef struct{
	int cost;
	int next;

}node;

node data[max_node][max_node];
node init[max_node][max_node];

int count;
FILE* out;

void OutputRout();
void updata();
void writemess(char* filename);
void exchange();
void initData(FILE * topo);
void commu(int recv, int send);

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
	
	
	//printf("start init\n");
	count = 0;
	initData(topo);                       // read in data from topo file
	fclose(topo);
	//printf("end init\n");
	
	for(i = 0; i <= count+2; i++){
		exchange();                      // do bellman-ford algorithm
	}

	OutputRout();                        // print forwarding table of routers  

	writemess(argv[2]);                  // write mess to output
	char tp[1024];
	int flag = 0;
	
	FILE * ch = fopen(argv[3], "r");
	while(fgets(tp, 1024, ch) != NULL){
		
		if(strlen(tp) < 2)
			break;
		
		int a = 0;
		int b = 0;
		int dist = 0;
		//char x[10];
		//char y[10];
		//char z[10];
		//printf("%s", tp);
		int kkk = sscanf(tp, "%d %d %d", &a, &b, &dist);
		//printf("what %d %d %d \n", a, b, dist);
		//if(data[a-1][b-1].cost >= dist || a >count || b>count){
			
		init[a-1][b-1].cost = dist;          // read in each change and update the distance between routers
		init[b-1][a-1].cost = dist;
		/*
		if(dist < 0){
			init[a-1][b-1].cost = -999;
			init[b-1][a-1].cost = -999;
		}
		*/
		init[a-1][b-1].next = b-1;           // also update the connected routers
		init[b-1][a-1].next = a-1;
		count = max(count, a);
		count = max(count, b);
		updata();                           
		for(i = 0; i <= count+2; i++){
			exchange();                    // do bellman-ford and get new forwarding table
		}
			//printf("%d\n", data[1][2].cost);
			//printf("%d\n", init[1][2].cost);
		//}
		OutputRout();                  // print forwarding table to output
		writemess(argv[2]);            // print message to output
		///memset(&tp, 0, sizeof(tp));
		 
	}
	fclose(out);
	fclose(ch);
	return 0;

}

void updata(){                             // update distance and connected router for each entry
	int i;
	int j;
	for(i = 0; i< count; i++){
		for(j = 0; j < count; j++){
			data[i][j].cost = init[i][j].cost;
			data[i][j].next = init[i][j].next;
		}
	}

}

void writemess(char* filename){
	FILE * mess = fopen(filename, "r");
	int start;
	int end;
	char temp[1024];
	while(fgets(temp, 1024, mess) != NULL){
															// read in each message
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
		char str[1024] = " ";
		int cur = start-1;
		//printf("%d\n", end);
		
	     												// find the path from start to end
		//printf("mes1\n");
		if(data[start-1][end-1].cost > 0){
			while(data[cur][end-1].cost >= 1){            ///
				//printf("what!\n");
				char s[100];
				sprintf(s, "%d ", cur+1);
				//printf("%s\n", s);
				//printf("%d\n", data[cur][end-1].next+1);
				strcat(str, s);
				cur = data[cur][end-1].next;
				//printf("%s\n", str);
			}
			//printf("end read\n");
			
																// print the path to output
			char buff[2048];
			sprintf(buff, "from %d to %d hops%smessage %s", start, end, str, msg);
			fwrite(buff, sizeof(char), strlen(buff), out);
		}
		
		else if(data[start-1][end-1].cost == 0){
			char s[100];
			sprintf(s, "%d ", cur+1);
			strcat(str, s);
			char buff[2048];
			sprintf(buff, "from %d to %d hops%smessage %s", start, end, str, msg);
			fwrite(buff, sizeof(char), strlen(buff), out);
			
		}
		else{
			char buff[2048];
			//printf("%s\n", msg);
			sprintf(buff, "from %d to %d hops unreachable message %s", start, end, msg);
			fwrite(buff, sizeof(char), strlen(buff), out);
		}
	}
	fclose(mess);
}


void initData(FILE * topo){
	ssize_t size = 0;
	char msg[1024];
	size_t read = 0;
	int i;
	int j;
	for(i = 0; i < max_node; i++){
		for(j = 0; j < max_node; j++){
			data[i][j].cost = -999;
			data[i][j].next = -1;
			init[i][j].cost = -999;
			init[i][j].next = -1;
			if(i == j){
				data[i][j].cost = 0;
				data[i][j].next = i;
				init[i][j].cost = 0;
				init[i][j].next = i;
			}
		}	
	}
											// read in topo file and init data information
	while(fgets( msg, 1024, topo) != NULL){
		int a;
		int b;
		int dist;
		sscanf(msg, "%d %d %d", &a, &b, &dist);
		//printf("%d\n", a);
		//printf("%d\n", b);
		//printf("%d\n", dist);
		data[a-1][b-1].cost = dist;
		data[b-1][a-1].cost = dist;
		data[a-1][b-1].next = b-1;
		data[b-1][a-1].next = a-1;
		
		init[a-1][b-1].cost = dist;
		init[b-1][a-1].cost = dist;
		init[a-1][b-1].next = b-1;
		init[b-1][a-1].next = a-1;
		/*
		if(dist < 0){
			data[a-1][b-1].cost = -999;
			data[b-1][a-1].cost = -999;
			init[a-1][b-1].cost = -999;
			init[b-1][a-1].cost = -999;
		}
		*/
		count = max(count, a);
		count = max(count, b);
	}
	
}

void exchange(){                                  // this function exchange forwarding table between routers
	int i;
	int j;
	for(i = 0; i < count; i++){
		for(j = 0; j < count; j++){
			if(data[i][j].cost > 0){
				commu(i, j);
			}	
		}
	}
}

void commu(int recv, int send){               // check for each entry in 2 forwarding table
	int i;
	for(i = 0; i < count; i++){
		if(data[send][i].cost >= 0){
			if(data[recv][i].cost < 0){
				data[recv][i].cost = data[send][i].cost + data[recv][send].cost;
				data[recv][i].next = data[recv][send].next;
			}
			else if(data[recv][i].cost > (data[send][i].cost + data[recv][send].cost)){
				data[recv][i].cost = data[send][i].cost + data[recv][send].cost;
				data[recv][i].next = data[recv][send].next;
				/*
				if(recv == 0 && i == 1){
					printf("%d\n", send);
					printf("%d\n", data[recv][i].cost);
					printf("-----");
				}
				*/
			}
			
			else if(data[recv][i].cost == data[send][i].cost + data[recv][send].cost){
				if(send < data[recv][i].next){
					data[recv][i].cost = data[send][i].cost + data[recv][send].cost;
					data[recv][i].next = data[recv][send].next;
				}
			}
			
		}
	}
}

void OutputRout(){            // print forwarding table of each router into output
	int i;
	int j;
	for(i = 0; i< count; i++){
		for(j = 0; j < count; j++){
			if(data[i][j].cost >= 0){
				char buff[100];
				sprintf(buff, "%d %d %d\n", j+1, data[i][j].next + 1, data[i][j].cost);
				fwrite(buff, sizeof(char), strlen(buff), out);
			}
			else{
				char buff[100];
				sprintf(buff, "%d -1 -1\n", j+1);
				fwrite(buff, sizeof(char), strlen(buff), out);
			}
		}
	}

}



