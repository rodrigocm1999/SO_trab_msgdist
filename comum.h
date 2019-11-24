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

#define true 1
#define false 0

#define MESSAGE_DURATION 5 // in seconds

#define LISTENER_PATH "/tmp/msgdist_listenerFifo"
#define USERNAME_L 32
#define TOPIC_L 20
#define TITLE_L 100
#define BODY_L 1000

//Protocol
#define SERVER_SHUTDOWN -10
#define KICKED -9
#define NEW_USER 10
#define USER_LEAVING 11
#define USERNAME_REPEATED 12
#define USERNAME_OK 13
#define NEW_MESSAGE 20
#define BAD_MESSAGE 21
#define MESSAGE_NOTIFICATION 22
#define GET_TOPICS 30
#define SUBSCRIBE_TOPIC 32
#define UNSUBSCRIBE_TOPIC 33
#define SUBSCRIBED_TO_TOPIC 34
#define ALREADY_SUBSCRIBED 35
#define NON_EXISTENT_TOPIC 36
#define HEARTBEAT_ISALIVE 40



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
	int beat; //bool
	char username[USERNAME_L];
	int fifo;
	LinkedList topics; // each node points to an existing char*
}User;

typedef struct NewClientInfo{
	pid_t pid;
	char username[USERNAME_L];
	char pathToFifo[64];
} NewClientInfo;

typedef struct Command{
	int cmd;
	pid_t senderPid;
	size_t structSize;
}Command;

typedef struct Buffer{
	void* ptr;
	unsigned int size;
}Buffer;

typedef struct MessageNotification{
	int id;
	char topic[TOPIC_L];
}MessageNotification;

Buffer joinCommandStruct(Command* command,void* other, size_t otherSize);

int isServerRunning();