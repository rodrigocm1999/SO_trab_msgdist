#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
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

int fileLock (char *fileSpec) {
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

void verifyBadWords(){
	char msgEnd[] = "##MSGEND##";
}

typedef struct {
	int id;
	char pathFIFO[32];
}NewClientInfo;

void* handleClient(void* data){
	NewClientInfo * info = data;
	
	//Dont forget to free it
	free(info);
}

void* waitForNewClients(void* data){
	//char mainFIFO[] = "./fifos/listener";
	char mainFIFO[] = "/tmp/gestorListenerFifo";
	int fifo = open(mainFIFO,O_RDONLY);
	if(fifo == -1){
		mkfifo(mainFIFO,0666);
		fifo = open(mainFIFO,O_RDONLY);
		if(fifo == -1 ){
			printf("Main client listener creation error (FIFO)\n");
			printf("Error: %d\n",errno);
			exit(0);
		}
	}
	int const bufferSize = 128;
	char buffer[bufferSize];
	while(1){
		read(fifo, buffer, bufferSize * sizeof(char));
		
		NewClientInfo * info = malloc(sizeof(NewClientInfo)); ;
		char* piece = strtok(buffer,",");
		info->id = atoi(piece);
		piece = strtok(buffer,",");
		strcpy(info->pathFIFO,piece);

		pthread_t thread;
		pthread_create(&thread,NULL,handleClient,(void*)info);
	}

}

void shutdown(int signal){
	printf("Exiting\n");
	exit(0);
}

int main(int argc,char* argv[]){

	//try to lock file to avoid multiple processes to run
	char filePath[100] =  "./lock.lock";
	int lock = fileLock(filePath);
	if (lock == -1){
		printf("Program already running\nExiting");
		exit(0);
	}

	//start verificador
	int verifPipe[2];
	pipe(verifPipe);
	int childPid = fork();
	if(childPid == 0){
		close(0);
		dup(verifPipe[0]);
		close(verifPipe[0]);
		close(verifPipe[1]);

		char badWordsFile[] = "badwords.txt";
		char ver[] = "verificador";
		execl(ver, ver, badWordsFile, (char*)NULL);
		printf("Bad Word Verifier Not started.");
		exit(0);
		//Exec gave error
	}

	signal(SIGINT, shutdown);

	pthread_t listenerThread;
	pthread_create(&listenerThread,NULL,waitForNewClients,(void*)NULL);

	Node* msgsHead = new_Node(NULL);
	Node* topicsHead = new_Node(NULL);
	Node* usersHead = new_Node(NULL);
	int filter = 0;
	/*List* msgsHead = new_List();
	List* topicsHead = new_List();
	List* usersHead = new_List();*/

	printf("Has started\n");

	printf("Write \"help\" to get command information\n");

	char command[512];
	char* cmd;
	while(1){
		scanf("%s",command);
		cmd = strtok(command," ");
		
		if(strcmp(cmd,"filter") == 0){
			char* token = strtok(NULL," ");
			printf("asddsa\n \tTest: %s \n",token);
			if(token != NULL){
				if(strcmp(token,"on") == 0){
					filter = 1;
					printf("Filter is on\n");
				}else if(strcmp(token,"off") == 0){
					filter = 0;
					printf("Filter is off\n");
				}else if(strcmp(token,"status") == 0){
					if(filter == 0) printf("Filter is off\n");
					else printf("Filter is on\n");
				}
			}else{
				printf("\nInvalid filter option, available : on off\n");
			}
			printf("asddsa 2\n");
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
					
				}

				curr = curr->next;
			}
		}


		else if(strcmp(cmd,"del") == 0){
			char* token = strtok(command," ");
		}


		else if(strcmp(cmd,"shutdown") == 0){
			shutdown(SIGINT);
		}


		else if(strcmp(cmd,"help") == 0){
			FILE* file = fopen("help.txt","r");
			int const bufferSize = 1024;
			char buffer[bufferSize];
			int sdas = fread(buffer,sizeof(char),bufferSize,file);
			printf("\n");
			write(0,buffer,strlen(buffer));
			printf("\n\n");
		}


	}
	return 0;
}
