#include "comum.h"
#include "utils.h"

#define DELIM " \n"

#define WORDSNOT "WORDSNOT"
#define DEFAULTWORDSNOT "./badwords.txt"
#define MAXNOT "MAXNOT"
#define DEFAULTMAXNOT 2

#define MSGEND "##MSGEND##\n"
#define MSGEND_L 11

Node* msgsHead;
Node* topicsHead;
Node* usersHead;


int sendToVerifier;
int recieveFromVerifier;
int maxbadWords;

int verifyBadWords(Message* message);
Message* new_Message();

int msgId = 0;

/*typedef struct {
	int pid;
	char username[64];
}NewClientInfo;*/




void* handleClient(void* data);

void printTopics(Node* head);
void printUsers(Node* head);
void printMsgs(Node* head);

void shutdown(int signal);