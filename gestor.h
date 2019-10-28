#include "comum.h"
#include "utils.h"

#define DELIM " \n"

void verifyBadWords();
Message* new_Message();

int msgId = 0;

typedef struct {
	int id;
	char pathFIFO[32];
}NewClientInfo;



void* handleClient(void* data);

void printTopics(Node* head);
void printUsers(Node* head);
void printMsgs(Node* head);
