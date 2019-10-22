#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include "utils.h"

typedef struct{
	char topic[20];
	char title[20];
	char body[1000];
	int duration;
}Message;



void verify();
int acquireLock (char *fileSpec);
void releaseLock (int lockFd);
void shutdown(){
	exit(0);
}

int main(int argc,char* argv[]){

	printf("Starting\n");

	char filePath[100] =  "./lock.lock";
	int lock = acquireLock(filePath);
	if (lock == -1){
		printf("Program already running\nExiting");
		exit(0);
	}

	printf("Has started\n");

	Node* msgs = new_Node(NULL);
	Node* topics = new_Node(NULL);
	Node* users = new_Node(NULL);

	int filter = 0;



	while(1) {pause();}

	return 0;
}

void verify(){
	int p[2];
	pipe(p);
	int pid = fork();

	if(pid == 0){
		//Child
		close(stdin);
		dup(p[0]);
		close(p[0]);
		close(p[1]);
	}else{
		//Parent
		close(p[0]);
		char str[100];
		fprintf(p[1],str,strlen(str));
	}
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
	printf("Topics");

}