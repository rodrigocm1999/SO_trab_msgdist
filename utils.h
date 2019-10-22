#include <stdio.h>

typedef struct{
	char** str;
	int size;
	int maxSize;
}String;


typedef struct Node{
	void* data;
	struct Node* next;
	struct Node* previous;
}Node;

Node* LinkedList_getNode(Node* head,void* obj);
void LinkedList_detachNode(Node* node);
void LinkedList_append(Node* head,Node* newNode);
Node* LinkedList_prepend(Node* head,Node* newNode);
Node* LinkedList_pop(Node* head,void* obj);
Node* LinkedList_getLast(Node* head);
Node* LinkedList_getHead(Node* node);
Node* new_Node(void* data);