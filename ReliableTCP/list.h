/** @file list.h */
#ifndef __LIST_H__
#define __LIST_H__

#include <time.h>


typedef struct _list_node{
	struct _list_node * prev;
	struct _list_node * next;
	char * data;
	//unsigned long int size;
	unsigned long int seq;
	unsigned int sent;
	unsigned int acks;

} list_node;



unsigned int list_node_get_seq(const list_node * node);
//unsigned int list_writeAckedToFile(list_node ** head, list_node ** tail,FILE* file);
void list_insert(list_node ** head, list_node ** tail, char * data, unsigned long int seq,unsigned long int seq2);

list_node* list_batchAck(list_node * head,unsigned long int seq);
list_node* list_Ack(list_node * head,unsigned long int seq);

#endif
