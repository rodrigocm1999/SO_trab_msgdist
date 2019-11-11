#include "gestor.h"


ServerConfig cfg;

int main(int argc,char* argv[]){	
	// Variable Initialization
	{
		cfg.msgId = 0;
		cfg.filter = 1;
		cfg.users.head = NULL;
		cfg.topics.head = NULL;
		cfg.msgs.head = NULL;
		int error = 0;
		error |= pthread_mutex_init(&cfg.mutex.msgsLock,NULL);
		error |= pthread_mutex_init(&cfg.mutex.topicsLock,NULL);
		error |= pthread_mutex_init(&cfg.mutex.usersLock,NULL);
		if(error != 0){
			printf("Mutex creation has failed\n");
			shutdown(SIGINT);
		}
	}
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
	{
		pthread_t listenerThread;
		pthread_create(&listenerThread,NULL,clientMessageReciever,(void*)NULL);
		pthread_t checkClientsThread;
		pthread_create(&listenerThread,NULL,checkAllClientsState,(void*)NULL);
		pthread_t checkMessageTimeoutThread;
		pthread_create(&listenerThread,NULL,checkMessageTimeout,(void*)NULL);
	}

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
				char* topic = strtok(NULL,DELIM);
				if(topic != NULL){
					if(getTopicNode(topic) == NULL){
						printf("Non existing topic : '%s'\n",topic);
					}else{
						int counter = 0;
						Node* curr = cfg.msgs.head;
						while( curr != NULL ){
							Message* currMessage = (Message*)curr->data;
							if(strcmp(currMessage->topic,topic) == 0){
								++counter;
								printf("Id : %d, Title : %s\n",currMessage->id,currMessage->title);	
							}
							curr = curr->next;
						}
						printf("Total %d\n",counter);
					}
				}else{
					printf("Invalid command usage\nExample : topic TOPICNAME\n");
				}
			}

			else if(strcmp(cmd,"del") == 0){
				char* token = strtok(NULL,DELIM);
				int id = atoi(token);
				if(id == 0){
					printf("Invalid Identifier\n");
				}
				else{
					int found = FALSE;
					Node* curr = cfg.msgs.head;
					while(curr != NULL){
						Message* message = (Message*)curr->data;
						if(message->id == id){
							LinkedList_detachNode(&cfg.msgs,curr);
							free(message);
							free(curr);
							printf("Deleted , id = \"%d\"\n",id);
							found = TRUE;
							break;
						}
						curr = curr->next;
					}
					if(found == FALSE){
						printf("Message with id = \"%d\" not found\n",id);
					}
				}
			}

			else if(strcmp(cmd,"kick") == 0){
				char* username = strtok(NULL,DELIM);
				if(username != NULL){
					Node* userNode = getUserNodeByUsername(username);
				
					if(userNode != NULL){
						sendToClient((User*)userNode->data,KICKED,NULL,0);
						userLeft(userNode);
					}else{
						printf("No user with name : '%s'\n",username);
					}
				}else{
					printf("Invalid command usage\nExample : kick USERNAME\n");
				}
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

			else {
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
		char buff[8192];
		void* buffer = buff;

		int bCount = read(fifo, buffer, 8192 * sizeof(char));
		Command* command = (Command*)buffer;
		buffer = buffer + sizeof(Command);
		//fprintf(stderr,"[INFO]Recebeu commando : %d , size : %zu , clientPid : %d\n", command->cmd,command->structSize,command->clientPid);

		switch (command->cmd){


			case NEW_USER:{
				NewClientInfo* info = (NewClientInfo*)buffer;
				User* newUser = malloc(sizeof(User));
				newUser->pid = info->pid;
				newUser->beat = TRUE;
				strcpy(newUser->username,info->username);

				int wasRepeated = FALSE;

				while(getUserNodeByUsername(newUser->username) != NULL){
					wasRepeated = TRUE;
					printf("Dupped name\n");

					// add number to username if dupped
					char* underline = &newUser->username[strlen(newUser->username)];
					int currentNumber = atoi(underline + 1);

					if(strncmp(underline,"_",1) == 0){ //if it has an underline
						sprintf(underline,"_%d",currentNumber + 1);
						printf("New number : %d\n",currentNumber + 1);
					}else{
						sprintf(underline,"_1");
					}
				}

				printf("[INFO]New Client; pid : %d , Username : %s\n",command->senderPid,newUser->username);

				int fd = open(info->pathToFifo,O_RDWR);
				if(fd == -1){
					fprintf(stderr,"[ERROR]Error opening user fifo : Descarting User\n");
				} else{
					fprintf(stderr,"[INFO]Opened client fifo\n");
					newUser->fifo = fd;
					addUser(newUser);

					if(wasRepeated){
						sendToClient(newUser,USERNAME_REPEATED,newUser->username,USERNAME_L);
					}else{
						sendToClient(newUser,USERNAME_OK,NULL,0);
					}
				}
				
				break;
			}


			case NEW_MESSAGE:{
				User* user = getUser(command->senderPid);
				Message* message = (Message*)buffer;
				int allowed = 1;
				if(cfg.filter){
					int badWordsCount = verifyBadWords(message);
					if(badWordsCount > cfg.maxbadWords){
						allowed = 0;
						fprintf(stderr,"[INFO]Message Descarded, user : '%s', bad words: %d\n",user->username,badWordsCount);
						sendToClient(user,BAD_MESSAGE,NULL,0);
						break;
					}
				}
				if(allowed){
					Message* realMessage = malloc(sizeof(Message));
					memcpy(realMessage,message,sizeof(Message));
					realMessage->id = ++cfg.msgId;
					realMessage->duration = MESSAGE_DURATION;

					addMessage(realMessage);
					Node* currTopic = cfg.topics.head;
					//check if topic exists
					int found = 0;
					while (currTopic != NULL){
						if(currTopic->data != NULL){
							char* topic = (char*)currTopic->data;
							if(strcmp(topic,realMessage->topic)==0){
								found = 1;
								break;
							}
						}
						currTopic = currTopic->next;
					}
					//add new topic if doesn't exist
					if(found == 0){
						char* topic = malloc(TOPIC_L);
						strcpy(topic,realMessage->topic);
						addTopic(topic);
					}


					// Send Notification to all subscribed clients
					MessageNotification notification;
					notification.id = realMessage->id;
					strncpy(notification.topic,realMessage->topic,TOPIC_L);
					Buffer buffer = prepareBuffer(MESSAGE_NOTIFICATION,&notification,sizeof(MessageNotification));

					Node* currUserNode = cfg.users.head;
					while(currUserNode != NULL){
						User* currUser = (User*) currUserNode->data;
						// If user is subscribed send notification
						if(getUserTopicNode(currUser,realMessage->topic) != NULL){
							sendBufferToClient(currUser,buffer);
						}
						currUserNode = currUserNode->next;
					}
					
					fprintf(stderr,"[INFO]New Message , user : '%s', id : '%d', title :'%s'\n",realMessage->username,realMessage->id,realMessage->title);
				}
				break;
			}


			case USER_LEAVING:{
				userLeft(getUserNode(command->senderPid));
				break;
			}


			case SUBSCRIBE_TOPIC:{
				User* user = getUser(command->senderPid);
				char* topic = buffer;
				Node* topicNode = getTopicNode(topic);
				char* topicChar = (char*)topicNode->data;
				
				if(topicNode != NULL){ // if topic exists
					Node* userTopic = getUserTopicNode(user,topicChar);
					if(userTopic == NULL){
						LinkedList_append(&user->topics,topicNode->data);
						fprintf(stderr,"[INFO]User \"%s\" subscribed to topic \"%s\"\n",user->username,topic);
						sendToClient(user,SUBSCRIBED_TO_TOPIC,NULL,0);
					}else{
						sendToClient(user,ALREADY_SUBSCRIBED,NULL,0);
					}
				}else{
				 	sendToClient(user,NON_EXISTENT_TOPIC,NULL,0);
				}
				break;
			}


			case UNSUBSCRIBE_TOPIC:{
				User* user = getUser(command->senderPid);
				char* topic = buffer;
				
				if(deleteUserTopic(user,topic) == TRUE){
					fprintf(stderr,"User \"%s\" unsubscribed to topic \"%s\"\n",user->username,topic);
					sendToClient(user,UNSUBSCRIBE_TOPIC,NULL,0);
				}else{
				 	sendToClient(user,NON_EXISTENT_TOPIC,NULL,0);
				}
				break;
			}


			case GET_TOPICS:{
				int topicsAmount = LinkedList_getSize(&cfg.topics);
				int totalBufferSize = sizeof(int) + topicsAmount * TOPIC_L;
				void* ptr = malloc(totalBufferSize);
				void* temp = ptr + sizeof(int);

				Node* curr = cfg.topics.head;
				for(int i = 0; i < topicsAmount && curr != NULL; i++){
					void* pos = temp + i * TOPIC_L;
					memcpy(pos,curr->data,TOPIC_L);
					curr = curr->next;
				}

				User* user = getUser(command->senderPid);
				sendToClient(user,GET_TOPICS,ptr,totalBufferSize);

				break;
			}


			case HEARTBEAT_ISALIVE:{
				User* user = getUser(command->senderPid);
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

void* checkAllClientsState(void* data){
	while(1){
		sleep(10);
		Node* curr = cfg.users.head;
		while(curr != NULL){
			User* user = (User*) curr->data;
			if(user->beat == FALSE){
				printf("User disconnected : %s\n",user->username);
				userLeft(curr);
			}
			user->beat = FALSE;
			curr = curr->next;
		}
	}
}

void* checkMessageTimeout(void* data){
	int const sleepTime = 1;
	while(TRUE){
		sleep(sleepTime);
		Node* curr = cfg.msgs.head;
		while(curr!=NULL){
			Message* message = (Message*)curr->data;
			Node* next = curr->next;
			message->duration = message->duration - sleepTime;
			if(message->duration < 0){
				//Delete message
				LinkedList_detachNode(&cfg.msgs,curr);
				free(curr->data);
				free(curr);
			}
			curr = next;
		}
	}
}

void userLeft(Node* node){
	LinkedList_detachNode(&cfg.users,node);
	// Limpar tudo acerca do utilizador
	User* user = (User*) node->data;
	// 1º limpar Nodes dos topicos a que pertence
	Node* currNode = user->topics.head;
	while(currNode != NULL){
		Node* nextnode = currNode->next;
		free(currNode);
		currNode = nextnode;
	}
	// 2º limpar o user
	close(user->fifo);
	free(user);
	// 3º limpar o node do user 
	free(node);
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

void printMsgs(Node* head){
	Node* curr = head;
	printf("Msgs on Memory : %d total\n", LinkedList_getSize(&cfg.msgs));
	
	while(curr != NULL){
		//TODO
		Message* currMessage = (Message*)curr->data;
		printf("Id : %d,\n\tTitle : %s,\n\tUsername : %s,\n\tTopic : %s\n",currMessage->id,currMessage->title,currMessage->username,currMessage->topic);
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
	command.senderPid = getpid();
	command.structSize = size;

	Buffer buffer = joinCommandStruct(&command,other,size);
	write(user->fifo,buffer.ptr,buffer.size);
	free(buffer.ptr);
}

Buffer prepareBuffer(int cmd,void* other, size_t size){
	Command command;
	command.cmd = cmd;
	command.senderPid = getpid();
	command.structSize = size;

	Buffer buffer = joinCommandStruct(&command,other,size);
	return buffer;
}

void sendBufferToClient(User* user,Buffer buffer){ //Used in loops
	write(user->fifo,buffer.ptr,buffer.size);
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

Node* getUserNodeByUsername(char* username){
	Node* curr = cfg.users.head;
	while(curr != NULL) {
		User* currUser = (User*)curr->data;
		if(strcmp(currUser->username,username) == 0){
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

User* getUserByUsername(char* username){
	return (User*) getUserNodeByUsername(username)->data;
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

int deleteUserTopic(User* user,char* topic){

	Node* userTopicNode = getUserTopicNode(user,topic);
	if(userTopicNode == NULL){
		return FALSE;
	}

	LinkedList_detachNode(&user->topics,userTopicNode);
	free(userTopicNode); 

	return TRUE;
}

void addUser(User* newUser){
	pthread_mutex_lock(&cfg.mutex.usersLock);
	LinkedList_append(&cfg.users,newUser);
	pthread_mutex_unlock(&cfg.mutex.usersLock);
}

void addTopic(char* newTopic){
	pthread_mutex_lock(&cfg.mutex.topicsLock);
	LinkedList_append(&cfg.topics,newTopic);
	pthread_mutex_unlock(&cfg.mutex.topicsLock);
}

void addMessage(Message* message){
	pthread_mutex_lock(&cfg.mutex.msgsLock);
	LinkedList_append(&cfg.msgs,message);
	pthread_mutex_unlock(&cfg.mutex.msgsLock);
}