#include "utils.h"


//LinkedList ----------------------------------------------------------


Node* LinkedList_getNode(LinkedList* list,void* obj){
	Node* curr = list->head;
	while(curr != NULL){
		if (curr->data == obj){
			return curr;
		}
		curr = curr->next;
	}
}

void LinkedList_append(LinkedList* list,void* newObj){
	if(list->head == NULL){
		list->head = new_Node(newObj);
		return;
	}
	Node* last = Node_getLast(list->head);
	Node* newNode = new_Node(newObj);
	last->next  = newNode;
	newNode->previous = last;
}
void LinkedList_appendNode(LinkedList* list,Node* newNode){
	if(list->head == NULL){
		list->head = newNode;
		return;
	}
	Node* last = Node_getLast(list->head);
	last->next  = newNode;
	newNode->previous = last;
}

void LinkedList_prepend(LinkedList* list,void* newObj){
	Node* newNode = new_Node(newObj);
	newNode->next = list->head;
	list->head = newNode;
}

Node* LinkedList_pop(LinkedList* list,void* obj){
	Node* node = Node_getLast(list->head);
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

void LinkedList_detachNode(LinkedList* list,Node* node){
	if(node == list->head){
		list->head = node->next;
		return;
	}
	Node_detach(node);
}

int LinkedList_getSize(LinkedList* list){
	int size = 0;
	Node* curr = list->head;
	while(curr != NULL){
		++size;
		curr = curr->next;
	}
	return size;
}
void LinkedList_prune(LinkedList* list){
	Node* curr = list->head;
	while(curr != NULL){
		if(curr->data == NULL){
			void* toDelete = curr;
			curr = curr->next;
			Node_detach(toDelete);
		}else{
			curr = curr->next;
		}
	}
}

//Node --------------------------------------------------------

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

Node* Node_getHead(Node* node){
	while(node->previous != NULL){
		node = node->previous;
	}
	return node;
}
Node* Node_getLast(Node* head){
	Node* curr = head;
	while(curr->next != NULL){
		curr = curr->next;
	}
	return curr;
}

void Node_detach(Node* node){
	if(node->previous != NULL){
		node->previous->next = node->next;
	}
	if(node->next != NULL){
		node->next->previous = node->previous;
	}
}





//ArrayList ---------------------------------------------

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