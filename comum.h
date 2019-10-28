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

//path to listener fifo
#define LISTENERPATH "/tmp/msgdist_listenerFifo"

#define NEW_USER 10
#define DEL_USER 11
#define NEW_MESSAGE 20
#define KILL_SERVER -10




typedef struct{
	int id;
	char topic[20];
	char title[100];
	char body[1000];
	int duration;
}Message;

typedef struct {
	pid_t pid;
	char* name;
	char** topics;
}User;



int isServerRunning();