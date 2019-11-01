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

#define LISTENERPATH "/tmp/msgdist_listenerFifo"
#define USERNAME_L 32
#define TOPIC_L 20
#define TITLE_L 100
#define BODY_L 1000

//Protocol
#define SERVER_SHUTDOWN -10
#define NEW_USER 10
#define USER_LEAVING 11
#define NEW_MESSAGE 20
#define GET_TOPICS 30
#define HEARTBEAT_CHECK 40
#define HEARTBEAT_OK 41



typedef struct Message{
	int id;
	char username[USERNAME_L];
	char topic[TOPIC_L];
	char title[TITLE_L];
	char body[BODY_L];
	int duration;
}Message;

typedef struct User{
	pid_t pid;
	char username[USERNAME_L];
	FILE* fifo;
	LinkedList topics;
}User;

typedef struct NewClientInfo{
	pid_t pid;
	char username[USERNAME_L];
	char pathToFifo[64];
} NewClientInfo;

typedef struct Command{
	int cmd;
	pid_t clientPid;
	char username[USERNAME_L];
	size_t structSize;
}Command;

typedef struct Pointer{
	void* ptr;
}Pointer;

typedef struct Buffer{
	void* ptr;
	unsigned int size;
}Buffer;

Buffer joinCommandStruct(Command* command,void* other, size_t otherSize);

int isServerRunning();