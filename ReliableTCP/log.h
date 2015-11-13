/** @file log.h */

#ifndef __LOG_H_
#define __LOG_H_



//typedef struct Node Node;


typedef struct Node{
    struct Node *next;
    struct Node *prev;
    char *data;
	int length;
    //int t_ack;/////
} Node;

typedef struct {
    Node* head;
    Node* tail;
} log_t;


void _Node(Node *a);
void log_init(log_t *l);
void log_destroy(log_t* l);
void log_push(log_t* l, const char *item, int size_a);
char *log_search(log_t* l, const char *prefix);
void clear(Node *l);
char *log_search_index(log_t* l, const int index);
void log_remove_head(log_t* l, const int index);


#endif
