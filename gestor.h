#include "comum.h"

#define DELIM " \n"

#define WORDSNOT "WORDSNOT"
#define DEFAULTWORDSNOT "./badwords.txt"
#define MAXNOT "MAXNOT"
#define DEFAULTMAXNOT 2

#define MSGEND "##MSGEND##\n"
#define MSGEND_L 11


typedef struct ServerConfig{
   int sendVerif;
   int recieveVerif;
   int filter;
   unsigned int maxbadWords;
   unsigned int msgId;
   LinkedList msgs;
   LinkedList topics;
   LinkedList users;
}ServerConfig;






int verifyBadWords(Message* message);
Message* new_Message();


void* checkAllClientsState(void* data);
void* clientMessageReciever(void* data);
void sendToClient(User* user,int cmd,void* other, size_t size);
Buffer prepareBuffer(int cmd,void* other, size_t size);
void sendBufferToClient(User* user,Buffer buffer);// Used in loops
void* heartBeat(void* data);

void printTopics(Node* head);
void printUsers(Node* head);
void printMsgs(Node* head);

void shutdown(int signal);
void userLeft(Node* node);

Node* getUserNode(pid_t pid);
User* getUser(pid_t pid);
Node* getUserNodeByUsername(char* username);
User* getUserByUsername(char* username);
Node* getTopicNode(char* topic);
Node* getUserTopicNode(User* user,char* topic);

int deleteUserTopic(User* user,char* topic);