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
int LinkedList_getSize(Node* node);
Node* new_Node(void* data);
int Node_hasNext(Node* node);

typedef struct List{
	void** arr;
	int size;
	int capacity;
}List;

List* new_List();
void List_add(List* list,void* newElem);
void List_expand(List* list);
void List_removeAt(List* list,int pos);
void List_shrink(List* list);

void Array_copyContent(void** arr,int size,void** other);

