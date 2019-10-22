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
	/*Node* curr = head;
	while(curr != NULL){
		if (curr->data == obj){
			if(curr->previous != NULL){
				curr->previous->next = curr->next;
			}
			if(curr->next != NULL){
				curr->next->previous = curr->previous;
			}
			return curr;
		}
		curr = curr->next;
	}*/
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

Node* new_Node(void* data){
	Node* ptr = malloc(sizeof(Node));
	ptr->data = data;
	ptr->next = NULL;
	ptr->previous = NULL;
	return ptr;
}