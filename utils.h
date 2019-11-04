#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
	void* data;
	struct Node* next;
	struct Node* previous;
}Node;

typedef struct LinkedList{
	void* head;
}LinkedList;

Node* LinkedList_getNode(LinkedList* list,void* obj);
void LinkedList_append(LinkedList* list,void* newObj);
void LinkedList_appendNode(LinkedList* list,Node* node);
void LinkedList_prepend(LinkedList* list,void* newObj);
Node* LinkedList_pop(LinkedList* list,void* obj);
void LinkedList_detachNode(LinkedList* list,Node* node);
int LinkedList_getSize(LinkedList* list);
void LinkedList_prune(LinkedList* list);

Node* new_Node(void* data);
int Node_hasNext(Node* node);
Node* Node_getHead(Node* node);
Node* Node_getLast(Node* node);
void Node_detach(Node* node);

typedef struct List{
	void** arr;
	int size;
	int capacity;
}List;

List* new_List();
void List_add(List* list,void* newElem);
void List_expand(List* list);
void List_removeAt(List* list,int pos);
void List_insertAt(List* list,int pos,void* obj);
int List_indexOf(List* list,void* obj);
void List_shrink(List* list);

void Array_copyContent(void** arr,int size,void** other);

