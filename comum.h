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

typedef struct{
	int id;
	char topic[20];
	char title[20];
	char body[1000];
	int duration;
}Message;

int isServerRunning();