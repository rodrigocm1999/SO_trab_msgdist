#include "comum.h"
#include "utils.h"

#define DELIM " \n"

#define WORDSNOT "WORDSNOT"
#define DEFAULTWORDSNOT "./badwords.txt"
#define MAXNOT "MAXNOT"
#define DEFAULTMAXNOT 2

#define MSGEND "##MSGEND##\n"
#define MSGEND_L 11


typedef struct{
   int sendVerif;
   int recieveVerif;
   unsigned int maxbadWords;
   unsigned int msgId;

}ServerConfig;


Node* msgsHead;
Node* topicsHead;
Node* usersHead;



int verifyBadWords(Message* message);
Message* new_Message();

void* clientMessageReciever(void* data);

void printTopics(Node* head);
void printUsers(Node* head);
void printMsgs(Node* head);

void shutdown(int signal);

void accquireLock();