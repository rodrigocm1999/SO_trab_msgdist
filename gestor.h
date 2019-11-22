#include "comum.h"

#define RECIEVE_BUFFER_SIZE 1024 * 16
#define DELIM " \n"

#define WORDSNOT "WORDSNOT"
#define DEFAULTWORDSNOT "./badwords.txt"
#define MAXNOT "MAXNOT"
#define DEFAULTMAXNOT 2

#define MSGEND "##MSGEND##\n"
#define MSGEND_L 11


struct listsMutex{
   pthread_mutex_t msgsLock;
   pthread_mutex_t topicsLock;
   pthread_mutex_t usersLock;
};

typedef struct ServerConfig{
   int sendVerif;
   int recieveVerif;
   int filter;
   unsigned int maxbadWords;
   unsigned int msgId;
   LinkedList msgs;
   LinkedList topics;
   LinkedList users;
   struct listsMutex mutex;
}ServerConfig;


int verifyBadWords(Message* message);

void* checkAllClientsState(void* data);
void* clientMessageReciever(void* data);
void* checkMessageTimeout(void* data);
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
Node *getMessageNodeById(int id);
Message *getMessageById(int id);
int deleteUserTopic(User* user,char* topic);
int messagesInTopic(char *topic);

void lock_m(pthread_mutex_t *mutex);
void unlock_m(pthread_mutex_t *mutex);
void lock_users(int bool);
void lock_msgs(int bool);
void lock_topics(int bool);
void lock_all(int bool);