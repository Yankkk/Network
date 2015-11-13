#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "tcp.h"

//forwards
list_node* list_batchAck(list_node * head,unsigned long int seq){
	list_node* curr=head;
	while(curr && curr->seq < seq){
		curr=curr->next;
	}
	if(curr){ 
		curr->acks=1;
		return curr->next;
	}else{
		return NULL;
	}
}

//backwards
list_node* list_Ack(list_node * head,unsigned long int seq){
	list_node* curr=head;
	while(curr && curr->seq > seq){
		curr=curr->prev;
	}
	return curr;
	
}



void list_insert(list_node ** head, list_node ** tail, char * data, unsigned long int seq,unsigned long int seq2)
{

	if(*tail==NULL){
		list_node * result = malloc(sizeof(list_node));

		result->data = NULL;
		result->next = NULL;
		result->seq = seq;
		result->prev = NULL;
		result->sent=0;
		result->acks=0;
		*tail = result;
		*head = result;
	}
	list_node * tmp=*head;

	//point the pointer to the corresponding node
	while(tmp->seq < seq2){
		if(tmp->next==NULL){
			tmp->next = malloc(sizeof(list_node));
			tmp->next->seq=tmp->seq+1;
			tmp->next->data=NULL;
			tmp->next->next=NULL;
			tmp->next->prev=tmp;
			tmp->sent=0;
			tmp->acks=0;
			*tail=tmp->next;		
		}
		tmp=tmp->next;

	}
	if(!tmp->data){
		tmp->data=data;
		//tmp->size=size;
	}

}

/*
unsigned int list_writeAckedToFile(list_node ** head, list_node ** tail,FILE* file)
{


	//printf("asdsad\n");
	list_node * cur = *head;	
	unsigned int new_seq=cur->seq;
	while(cur && cur->data ){
		list_node * temp = cur;
		cur = cur->next;
		new_seq++;

		fwrite(temp->data,sizeof(char),strlen(temp->data),file);
	//	printf("%s\n",temp->data);		
		//printf("%d\n",strlen(temp->data));

		free(temp->data);
		free(temp);

	}

	if(!cur){
		*head = NULL;
		*tail = NULL;
		return new_seq;
	}
	cur->prev = NULL;
	*head = cur;
	return new_seq;
}

*/

unsigned int list_node_get_seq(const list_node * node)
{
	tcp_t *header = (tcp_t *)(node->data);
	return header->seq;
}


