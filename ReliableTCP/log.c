/** @file log.c */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log.h"

/**
 * Initializes the log.
 *
 * You may assuem that:
 * - This function will only be called once per instance of log_t.
 * - This function will be the first function called per instance of log_t.
 * - All pointers will be valid, non-NULL pointer.
 *
 * @returns
 *   The initialized log structure.
 */
void log_init(log_t *l) {
    (*l).head=NULL;
    (*l).tail=NULL;
}

/**
 * Frees all internal memory associated with the log.
 *
 * You may assume that:
 * - This function will be called once per instance of log_t.
 * - This funciton will be the last function called per instance of log_t.
 * - All pointers will be valid, non-NULL pointer.
 *
 * @param l
 *    Pointer to the log data structure to be destoryed.
 */
void log_destroy(log_t* l) {
	if(l->head!=NULL)
    		clear(l->head);
}

/**
 * Push an item to the log stack.
 *
 *
 * You may assume that:
* - All pointers will be valid, non-NULL pointer.
*
 * @param l
 *    Pointer to the log data structure.
 * @param item
 *    Pointer to a string to be added to the log.
 */
void log_push(log_t* l, const char *item, int size_a) {//need free
	if (item==NULL)//check null
		return;    
	Node* a=malloc(sizeof(Node));
    _Node(a);
    //a->t_ack = 0;     
    a->data=malloc(sizeof(char)*size_a);//sizeof(*item)*2000);//*item=item[0],it's one char length
    //strncpy(a->data,item,size_a);// Warning: If there is no null byte among the first n bytes of src, the string placed in dest will not be null-terminated.
	memcpy(a->data, item, size_a);
	a->length = size_a;
	/*
	if(size_a < 1024){
		a->data[size_a]='\0';
	}
	*/
	if(l->head==NULL){
        l->head=a;
        l->tail=a;
    }
    else{
        l->tail->next=a;
        a->prev=l->tail;
        l->tail=a;
    }
	//printf("in_log data:\n%s\n",a->data);
}


/**
 * Preforms a newest-to-oldest search of log entries for an entry matching a
 * given prefix.
 *
 * This search starts with the most recent entry in the log and
 * compares each entry to determine if the query is a prefix of the log entry.
 * Upon reaching a match, a pointer to that element is returned.  If no match
 * is found, a NULL pointer is returned.
 *
 *
 * You may assume that:
 * - All pointers will be valid, non-NULL pointer.
 *
 * @param l
 *    Pointer to the log data structure.
 * @param prefix
 *    The prefix to test each entry in the log for a match.
 *
 * @returns
 *    The newest entry in the log whose string matches the specified prefix.
 *    If no strings has the specified prefix, NULL is returned.
 */
char *log_search(log_t* l, const char *prefix) {
    Node *current=l->tail;
    while(current){
        size_t length=strlen(prefix);
        if(strncmp(current->data,prefix,length)==0)
            return current->data;
        current=current->prev;
    }
    return NULL;
}

void clear(Node *l){
    if(l->next!=NULL)
         clear(l->next);
    free(l->data);
    free(l);
}

void _Node(Node *a){
    a->next=NULL;
    a->prev=NULL;
    a->data=NULL;
}


/**
 * Return the Node at some index.
 * 
 * @param l
 *    Pointer to the log data structure.
 * @param index
 *    The index of the entry.
 *
 * @returns
 *    The node at the index.
 *    If index is invalid, NULL is returned.
 */
char *log_search_index(log_t* l, const int index){
	if( l -> head == NULL)
		return NULL;
	Node * cur = l -> head;
	int i;
	for(i = 0; (i <= index) && (cur != NULL) ; i++){		
		if( index == i ){
			return cur -> data;	
		}
		cur = cur -> next;
	}
	return NULL;
}

/* remove head up to some point
 *
 */
void log_remove_head(log_t* l, const int index){
	if( l -> head == NULL)
		return;
	/*	
	printf("it did something\n");

	Node* k = l ->head;


	for(;k!=NULL;k = k ->next){
		printf("--------\n%s\n",k ->data);
	}	
	*/

	Node* cur = l -> head;
	int i;
	for(i = 0; (i <= index) && (cur != NULL); i++){
		l -> head = cur -> next;
		//clear(cur);
		free(cur -> data);
		free(cur);
		cur = l -> head;
	}
	/*
	k = l ->head;


	for(;k!=NULL;k = k ->next){
		printf("--------\n%s\n",k ->data);
	}
	*/
	return;
}






