#include <stdio.h>
#include <stdlib.h>
#include "utils.h"


Node* LinkedList_getNode(Node* head,void* obj){
	Node* curr = head;
	while(curr != NULL){
		if (curr->data == obj){
			return curr;
		}
		curr = curr->next;
	}
}

void LinkedList_detachNode(Node* node){
	if(node->previous != NULL){
		node->previous->next = node->next;
	}
	if(node->next != NULL){
		node->next->previous = node->previous;
	}
}

void LinkedList_append(Node* head,Node* newNode){
	Node* last = LinkedList_getLast(head);
	last->next = newNode;
	newNode->previous = last;
}

Node* LinkedList_prepend(Node* head,Node* newNode){
	Node* temp = head;
	head = newNode;
	newNode->next = temp;
	return head;
}

Node* LinkedList_pop(Node* head,void* obj){
	Node* node = LinkedList_getLast(head);
	if(node != NULL){
		if(node->previous != NULL){
			node->previous->next = node->next;
		}
		if(node->next != NULL){
			node->next->previous = node->previous;
		}
	}
	return node;
}

Node* LinkedList_getLast(Node* head){
	Node* curr = head;
	while(curr->next != NULL){
		curr = curr->next;
	}
	return curr;
}
Node* LinkedList_getHead(Node* node){
	while(node->previous != NULL){
		node = node->previous;
	}
	return node;
}
int LinkedList_getSize(Node* node){
	int size = 1;
	Node* curr = node;
	while(curr->next != NULL){
		++size;
		curr = curr->next;
	}
	return size;
}
void LinkedList_prune(Node* node){
	Node* curr = node;
	while(curr != NULL){
		if(curr->data == NULL){
			void* toDelete = curr;
			curr = curr->next;
			LinkedList_detachNode(toDelete);
		}else{
			curr = curr->next;
		}
	}
}

Node* new_Node(void* data){
	Node* ptr = malloc(sizeof(Node));
	ptr->data = data;
	ptr->next = NULL;
	ptr->previous = NULL;
	return ptr;
}

int Node_hasNext(Node* node){
	return node->next != NULL ? 1 : 0;
}

#define INITIALARRAYSIZE 4
#define ARRAYEXPANDMULTIPLIER 2

List* new_List(){
	List* obj = malloc(sizeof(List));
	obj->arr = malloc(INITIALARRAYSIZE * sizeof(void*));
	obj->capacity = 10;
	obj->size = 0;
}
void List_add(List* list,void* newElem){
	if(list->size == list->capacity){
		List_expand(list);
	}
}
void List_expand(List* list){
	int newCapacity = list->capacity * ARRAYEXPANDMULTIPLIER;
	void* newArr = malloc(newCapacity * sizeof(void*));
	Array_copyContent(list->arr,list->size,newArr);
	free(list->arr);
	list->arr = newArr;
	list->capacity = newCapacity;
}
void List_shrink(List* list){
	int newCapacity = list->size;
	void* newArr = realloc(list->arr,newCapacity * sizeof(void*));
	//Array_copyContent(list->arr,list->size,newArr);
	//free(list->arr);
	//list->arr = newArr;
	list->capacity = newCapacity;
}

void List_removeAt(List* list,int pos){
	printf("List_removeAt TODO");
}

void List_insertAt(List* this,int pos,void* obj){
	if(this->size >= this->capacity){
		List_expand(this);
	}
	void** arr = this->arr;
	void* temp;
	void* previous = obj;
	for(int i = pos; i < this->size; i++){
		temp = arr[i];
		arr[i] = previous;
		previous = temp;
	}
}

int List_indexOf(List* list,void* obj){
	void** arr = list->arr;
	for(int i = 0; i < list->size; i++){
		if(arr[i] == obj){
			return i;
		}
	}
	return -1;
}


void Array_copyContent(void** arr,int size,void** other){
	for(int i = 0; i< size; i++){
		other[i] = arr[i];
 	}
}