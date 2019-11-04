#include "gestor.h"


ServerConfig cfg;

int main(int argc,char* argv[]){	
	cfg.users.head = NULL;
	cfg.topics.head = NULL;
	cfg.msgs.head = NULL;
	//check for arguments and already running server
	{
		int checkServerRunning = 1;
		int res;
		while ((res = getopt(argc,argv,"f")) != -1){
			switch (res)
			{
			case 'f':
				printf("[INFO] Using force start option\n");
				if(isServerRunning()){
					printf("[INFO] Deleting old listener FIFO\n");
					unlink(LISTENER_PATH);
				}
				checkServerRunning = 0;
				break;
			
			default:
				printf("Unsuported Option\n-f = force start (used after crash or kill)\n");
				break;
			}

		}
		//check for already running server
		if(checkServerRunning && isServerRunning()){
			printf("Program already running\nExiting\n");
			exit(0);
		}
	}

	//Start words verifier
	{
		int sendPipe[2];
		pipe(sendPipe);
		int recievePipe[2];
		pipe(recievePipe);
		
		char* badWordsFile = getenv(WORDSNOT);
		if(badWordsFile == NULL){
			badWordsFile = DEFAULTWORDSNOT;
		}
		printf("[INFO] WORDSNOT = %s\n",badWordsFile);

		int childPid = fork();
		if(childPid == 0){
			close(0);
			dup(sendPipe[0]);
			close(sendPipe[0]);
			close(sendPipe[1]);

			close(1);
			dup(recievePipe[1]);
			close(recievePipe[0]);
			close(recievePipe[1]);

			//start verifier on child process			
			char ver[] = "verificador";
			execl(ver, ver, badWordsFile, (char*)NULL);
			fprintf(stderr,"[ERROR] Bad Word Verifier Not started.\n");
			exit(0);
			//Exec gave error
		} else {
			close(sendPipe[0]);
			close(recievePipe[1]);
		}
		cfg.sendVerif = sendPipe[1];
		cfg.recieveVerif = recievePipe[0];
	}

	//Max bad Words
	{
		char* temp = getenv(MAXNOT);
		if (temp != NULL){
			cfg.maxbadWords = atoi(temp);
		}else{
			cfg.maxbadWords = DEFAULTMAXNOT;
		}
		printf("[INFO] MAXNOT = %d\n",cfg.maxbadWords);
	}
	
	// Start listener for messages
	pthread_t listenerThread;
	pthread_create(&listenerThread,NULL,clientMessageReciever,(void*)NULL);
	pthread_t checkClientsThread;
	pthread_create(&listenerThread,NULL,checkAllClientsState,(void*)NULL);


	// Ready config variables
	cfg.msgId = 0;
	cfg.filter = 1;
	// Change Default Signal Effect
	signal(SIGINT, shutdown);

	printf("Write \"help\" to get command information\n");

	char command[512];
	char* cmd;
	while(1){
		printf("-> ");
		fgets(command,512,stdin);
		cmd = strtok(command,DELIM);
		if(cmd != NULL){

			if(strcmp(cmd,"filter") == 0){
				char* token = strtok(NULL,DELIM);
				if(token != NULL){
					if(strcmp(token,"on") == 0){
						cfg.filter = 1;
						printf("Filter is on\n");
					}else if(strcmp(token,"off") == 0){
						cfg.filter = 0;
						printf("Filter is off\n");
					}else if(strcmp(token,"status") == 0){
						if(cfg.filter == 0) printf("Filter is off\n");
						else printf("Filter is on\n");
					}
				}else{
					printf("\nInvalid filter option, available : on off status\n");
				}
			}

			else if(strcmp(cmd,"users") == 0){
				printUsers(cfg.users.head);
			}

			else if(strcmp(cmd,"topics") == 0){
				printTopics(cfg.topics.head);
			}

			else if(strcmp(cmd,"msg") == 0){
				printMsgs(cfg.msgs.head);
			}

			else if(strcmp(cmd,"topic") == 0){
				char* topic = strtok(command,DELIM);
				Node* curr = cfg.msgs.head;
				while( curr != NULL ){
					Message* currMessage = (Message*)curr->data;
					if(strcmp(currMessage->topic,topic) == 0){
						printf("Id : %d, Title : %s\n",currMessage->id,currMessage->title);	
					}
					curr = curr->next;
				}
			}

			else if(strcmp(cmd,"del") == 0){//TODO
				char* token = strtok(NULL,DELIM);
			}

			else if(strcmp(cmd,"kick") == 0){//TODO
				char* username = strtok(NULL,DELIM);
			}	

			else if(strcmp(cmd,"shutdown") == 0){
				shutdown(SIGINT);
			}

			else if(strcmp(cmd,"prune") == 0){ //TODO
				
			}

			else if(strcmp(cmd,"help") == 0){
				FILE* file = fopen("help.txt","r");
				int const bufferSize = 2048;
				char buffer[bufferSize];
				int sdas = fread(buffer,sizeof(char),bufferSize,file);
				printf("\n");
				write(0,buffer,strlen(buffer));
				printf("\n\n");
			}

			else if(strcmp(cmd,"verify") == 0){
				char* token = strtok(NULL,DELIM);
				while(token != NULL){
					write(cfg.sendVerif,token,strlen(token));
					write(cfg.sendVerif,"\n",1);
					token = strtok(NULL,DELIM);
				}
				write(cfg.sendVerif,MSGEND,MSGEND_L);
				char buffer[4];
				read(cfg.recieveVerif,buffer,4);
				int nBadWords = atoi(buffer);
				printf("Number of bad words : %d\n",nBadWords);
			}

			else { // Done
				printf("Write \"help\" to get command information\n");
			}
		}
	}
	return 0;
}


// TODO
void* clientMessageReciever(void* data){
	
	int result = mkfifo(LISTENER_PATH,0666);
	if(result != 0) {
		fprintf(stderr,"[ERROR]Creating listener fifo : already exists\n");
		exit(0);
	}
	int fifo = open(LISTENER_PATH,O_RDWR);
	if(fifo == -1 ){
		printf("[ERROR]Main client listener creation error (FIFO)\n");
		fprintf(stderr,"[ERROR]Main client listener creation error (FIFO)\n");
		exit(0);	
	}

	
	while(1){
		int const bufferSize = 8192;
		char buff[bufferSize];
		void* buffer = buff;

		int bCount = read(fifo, buffer, bufferSize * sizeof(char));
		fprintf(stderr,"[INFO]Recebeu bytes : %d\n",bCount);
		Command* command = (Command*)buffer;
		buffer = buffer + sizeof(Command);


		fprintf(stderr,"[INFO]Recebeu commando : %d \n\tsize : %zu , clientPid : %d\n",
			command->cmd,command->structSize,command->clientPid);

		switch (command->cmd){


			case NEW_USER:{ // TODO check if another user with same name exists
				NewClientInfo* info = (NewClientInfo*)buffer;
				User* user = malloc(sizeof(User));
				user->pid = info->pid;
				user->beat = TRUE;
				strcpy(user->username,info->username);
				printf("[INFO]Client fifo : %s, Username : %s\n",info->pathToFifo,user->username);

				int fd = open(info->pathToFifo,O_RDWR);
				if(fd == -1){
					fprintf(stderr,"[ERROR]Error opening user fifo : Descarting User\n");
				} else{
					fprintf(stderr,"[INFO]Opened client fifo\n");
					user->fifo = fd;
					LinkedList_append(&cfg.users,user);
				}
				
				break;
			}


			case NEW_MESSAGE:{			
				Message* message = (Message*)buffer;
				int allowed = 1;
				if(cfg.filter){
					int badWordsCount = verifyBadWords(message);
					if(badWordsCount > cfg.maxbadWords){
						allowed = 0;
						fprintf(stderr,"Message Descarded, too many bad words: %d\n",badWordsCount);
						sendToClient(getUser(command->clientPid),BAD_MESSAGE,NULL,0);
					}
				}
				if(allowed){
					Message* realMessage = malloc(sizeof(Message));
					memcpy(realMessage,message,sizeof(message));
					realMessage->id = ++cfg.msgId;

					LinkedList_append(&cfg.msgs,message);
					Node* currTopic = cfg.topics.head;
					//check if topic exists
					int found = 0;
					while (currTopic != NULL){
						if(currTopic->data != NULL){
							char* topic = (char*)currTopic->data;
							if(strcmp(topic,message->topic)==0){
								found = 1;
								break;
							}
						}
						currTopic = currTopic->next;
					}
					//add new topic if doesn't exist
					if(found == 0){
						char* topic = malloc(TOPIC_L);
						strcpy(topic,message->topic);
						LinkedList_append(&cfg.topics,topic);
					}
				}
				break;
			}


			case USER_LEAVING:{
				//TODO
				printf("Not Yet Implemented");

				break;
			}


			case SUBSCRIBE_TOPIC:{
				User* user = getUser(command->clientPid);
				char* topic = buffer;
				Node* topicNode = getTopicNode(topic);
				
				if(topicNode != NULL){ // if topic exists
					Node* userTopic = getUserTopicNode(user,topic);
					if(userTopic == NULL){
						LinkedList_append(&user->topics,topicNode->data);
					}
				}
				//TODO warn user of what happened

				break;
			}


			case HEARTBEAT_ISALIVE:{
				User* user = getUser(command->clientPid);
				user->beat = TRUE;				
				break;
			}


			default:{
				printf("Not Recognized Command\n");
				break;
			}
		}
	}

}

// TODO
void* checkAllClientsState(void* data){
	//Every ten seconds check if users have TODO 
	while(1){
		sleep(10);
		Node* curr = cfg.users.head;
		while(curr != NULL){
			User* user = (User*) curr->data;
			if(user->beat == FALSE){
				printf("User disconnected : %s\n",user->username);
				LinkedList_detachNode(&cfg.users,curr);
				// Limpar tudo acerca do utilizador
				{
					// 1º limpar Nodes dos topicos a que pertence
					Node* currNode = user->topics.head;
					while(currNode != NULL){
						Node* nextnode = currNode->next;
						free(currNode);
						currNode = nextnode;
					}
					// 2º limpar o user
					free(user);
					// 3º limpar o node do user 
					free(curr);
				}
			}
			user->beat = FALSE;
			curr = curr->next;
		}
	}
}


int verifyBadWords(Message* message){
	write(cfg.sendVerif,message->title,strlen(message->title));
	write(cfg.sendVerif,"\n",1);
	write(cfg.sendVerif,message->topic,strlen(message->topic));
	write(cfg.sendVerif,"\n",1);
	write(cfg.sendVerif,message->body,strlen(message->body));
	write(cfg.sendVerif,"\n",1);
	write(cfg.sendVerif,MSGEND,MSGEND_L);

	int const bufferSize = 4;
	char buffer[bufferSize];
	read(cfg.recieveVerif,buffer,bufferSize * sizeof(char));

	int nBadWords = atoi(buffer);
	return nBadWords;
}

void printTopics(Node* head){
	Node* curr = head;
	printf("Topics : %d total\n", LinkedList_getSize(&cfg.topics));
	
	while(curr != NULL){
		//TODO
		printf("\t%s , subscribers : \n",(char*)curr->data);
		curr = curr->next;
	}
}

void printUsers(Node* head){
	Node* curr = head;
	printf("Users online : %d total\n", LinkedList_getSize(&cfg.users) );
	
	while(curr != NULL){
		User* user = (User*)curr->data;
		printf("\t%s , subscribed to : ",user->username);

		Node* currTopic = user->topics.head;
		while( currTopic != NULL )
		{
			printf(", %s",(char*)currTopic->data);
			currTopic = currTopic->next;
		}
		printf("\n");

		curr = curr->next;
	}
}
// TODO
void printMsgs(Node* head){
	Node* curr = head;
	printf("Msgs on Memory : %d total\n", LinkedList_getSize(&cfg.msgs));
	
	while(curr != NULL){
		//TODO
		Message* currMessage = (Message*)curr->data;
		printf("Title : %s,\n\tUsername : %s,\n\tTopic : %s\n",currMessage->title,currMessage->username,currMessage->topic);
		curr = curr->next;
	}
}

void shutdown(int signal){
	printf("Exiting\n");
	unlink(LISTENER_PATH);
	Node* curr = cfg.users.head;
	while(curr != NULL){
		User* currUser = (User*)curr->data;
		sendToClient(currUser,SERVER_SHUTDOWN,NULL,0);
		printf("Sent shutdown to : %s\n",currUser->username);
		curr = curr->next;
	}
	exit(0);
}

void sendToClient(User* user,int cmd,void* other, size_t size){
	Command command;
	command.cmd = cmd;
	command.clientPid = getpid();
	command.structSize = size;

	Buffer buffer = joinCommandStruct(&command,other,size);
	int written = write(user->fifo,buffer.ptr,buffer.size);
	free(buffer.ptr);
}

Node* getUserNode(pid_t pid){
	Node* curr = cfg.users.head;
	while(curr != NULL) {
		User* currUser = (User*)curr->data;
		if(currUser->pid == pid){
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

User* getUser(pid_t pid){
	return (User*) getUserNode(pid)->data;
}

Node* getTopicNode(char* topic){
	Node* curr = cfg.topics.head;
	while(curr != NULL) {
		char* currTopic = (char*) curr->data;
		if(strcmp(currTopic,topic) == 0){
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

Node* getUserTopicNode(User* user,char* topic){
	Node* curr = user->topics.head;
	while(curr != NULL) {
		char* currTopic = (char*)curr->data;
		if(strcmp(currTopic,topic) == 0){
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}