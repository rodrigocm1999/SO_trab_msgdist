#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <pthread.h>
#include "utils.h"

typedef struct{
	int id;
	char topic[20];
	char title[20];
	char body[1000];
	int duration;
}Message;
int msgId = 0;

Message* new_Message(){
	Message* obj = malloc(sizeof(Message));
	obj->id = ++msgId;

	return obj;
}

int acquireLock (char *fileSpec) {
    int lockFd;

    if ((lockFd = open (fileSpec, O_CREAT | O_RDWR, 0600))  < 0){
        return -1;
	}
    if (flock (lockFd, LOCK_EX | LOCK_NB) < 0) {
        close (lockFd);
        return -1;
    }

    return lockFd;
}

void releaseLock (int lockFd) {
    flock (lockFd, LOCK_UN);
    close (lockFd);
}

void printTopics(Node* head){
	Node* curr = head;
	printf("Topics : %d total\n", LinkedList_getSize(head));
	
	while(curr != NULL){
		//TODO
		printf("\t%s , subscribers : \n",(char*)curr->data);
		curr = curr->next;
	}
}

void printUsers(Node* head){
	Node* curr = head;
	printf("Users online : %d total\n", LinkedList_getSize(head) );
	
	while(curr != NULL){
		//TODO
		printf("\t%s , subscribed to : \n",(char*)curr->data);
		curr = curr->next;
	}
}

void printMsgs(Node* head){
	Node* curr = head;
	printf("Msgs on Memory : %d total\n", LinkedList_getSize(head) );
	
	while(curr != NULL){
		//TODO
		printf("\t%s ,  : \n",(char*)curr->data);
		curr = curr->next;
	}
}

int main(int argc,char* argv[]){

	printf("Starting\n");

	char filePath[100] =  "./lock.lock";
	int lock = acquireLock(filePath);
	if (lock == -1){
		printf("Program already running\nExiting");
		exit(0);
	}

	int verifPipe[2];
	pipe(verifPipe);
	int childPid = fork();

	if(childPid == 0){
		//Child
		close(0);
		dup(verifPipe[0]);
		close(verifPipe[0]);
		close(verifPipe[1]);

		execl("./verificador","verificador",NULL);
	}else{
		//Parent
		close(verifPipe[0]);
		char str[100];
		fprintf(verifPipe[1],str,strlen(str));
	}


	pthread_t mainThread;
	pthread_create(mainThread,NULL,/*function pointer*/NULL,
	/*void* of the fucntion*/NULL);


	printf("Has started\n");

	Node* msgsHead = new_Node(NULL);
	Node* topicsHead = new_Node(NULL);
	Node* usersHead = new_Node(NULL);
	/*List* msgsHead = new_List();
	List* topicsHead = new_List();
	List* usersHead = new_List();*/

	int filter = 0;

	printf("Write \"help\" to get command information\n");

	char command[512];
	char* cmd;
	while(1){
		scanf("%s",command);
		cmd = strtok(command," ");
		
		if(strcmp(cmd,"filter") == 0){
			char* token = strtok(command," ");
			if(strcmp(token,"on") == 0){
				filter = 1;
			}else if(strcmp(token,"off") == 0){
				filter = 0;
			}else{
				printf("\nInvalid filter option, available : on off\n");
			}
		}
		if(strcmp(cmd,"users") == 0){
			printUsers(usersHead);
		}
		else if(strcmp(cmd,"topics") == 0){
			printUsers(topicsHead);
		}
		else if(strcmp(cmd,"msg") == 0){
			printMsgs(msgsHead);
		}
		else if(strcmp(cmd,"topic") == 0){
			char* topic = strtok(command," ");
			Node* curr = msgsHead;
			while( curr != NULL ){
				Message* currMessage = (Message*)curr->data;
				if(strcmp(currMessage->topic,topic) == 0){
					printf("");
				}

				curr = curr->next;
			}

		}
		else if(strcmp(cmd,"del")){
			char* token = strtok(command," ");
		}
		else if(strcmp(cmd,"shutdown")){
			printf("Exiting\n");
			exit(0);	
		}
		else if(strcmp(cmd,"help")){
			//open and print help.txt
		}

	}

	while(1) {pause();}
	return 0;
}
