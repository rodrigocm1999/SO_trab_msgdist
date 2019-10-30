#include "gestor.h"

ServerConfig cfg;

int main(int argc,char* argv[]){
		
	//check if process already running
	if (isServerRunning()){
		printf("Program already running\nExiting\n");
		exit(0);
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
		printf("[INFO]WORDSNOT = %s\n",badWordsFile);

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
			printf("[ERROR]Bad Word Verifier Not started.");
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
		printf("[INFO]MAXNOT = %d\n",cfg.maxbadWords);
	}
	// Start listener for messages
	pthread_t listenerThread;
	pthread_create(&listenerThread,NULL,clientMessageReciever,(void*)NULL);


	// Create lists for memory objects
	msgsHead = new_Node(NULL);
	topicsHead = new_Node(NULL);
	usersHead = new_Node(NULL);
	int filter = 0;
	/*List* msgs = new_List();
	List* topics = new_List();
	List* users = new_List();*/

	signal(SIGINT, shutdown);

	printf("Write \"help\" to get command information\n");

	char command[512];
	char* cmd;
	while(1){
		printf("-> ");
		fgets(command,512,stdin);
		cmd = strtok(command,DELIM);
		
		if(strcmp(cmd,"filter") == 0){
			char* token = strtok(NULL,DELIM);
			if(token != NULL){
				if(strcmp(token,"on") == 0){
					filter = 1;
					printf("Filter is on\n");
				}else if(strcmp(token,"off") == 0){
					filter = 0;
					printf("Filter is off\n");
				}else if(strcmp(token,"status") == 0){
					if(filter == 0) printf("Filter is off\n");
					else printf("Filter is on\n");
				}
			}else{
				printf("\nInvalid filter option, available : on off status\n");
			}
		}

		else if(strcmp(cmd,"users") == 0){
			printUsers(usersHead);
		}

		else if(strcmp(cmd,"topics") == 0){
			printUsers(topicsHead);
		}

		else if(strcmp(cmd,"msg") == 0){
			printMsgs(msgsHead);
		}

		else if(strcmp(cmd,"topic") == 0){
			char* topic = strtok(command,DELIM);
			Node* curr = msgsHead;
			while( curr != NULL ){
				Message* currMessage = (Message*)curr->data;
				if(strcmp(currMessage->topic,topic) == 0){
					
				}

				curr = curr->next;
			}
		}

		else if(strcmp(cmd,"del") == 0){
			char* token = strtok(command,DELIM);
		}

		else if(strcmp(cmd,"kick") == 0){
			char* username = strtok(command,DELIM);
		}	

		else if(strcmp(cmd,"shutdown") == 0){
			shutdown(SIGINT);
		}

		else if(strcmp(cmd,"prune") == 0){
			
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

		else {
			printf("Write \"help\" to get command information\n");
		}

	}
	return 0;
}

//Done
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
	printf("Topics : %d total\n", LinkedList_getSize(head));
	
	while(curr != NULL){
		//TODO
		printf("\t%s , subscribers : \n",(char*)curr->data);
		curr = curr->next;
	}
}

void printUsers(Node* head){
	Node* curr = head;
	printf("Users online : %d total\n", LinkedList_getSize(head) );
	
	while(curr != NULL){
		//TODO
		printf("\t%s , subscribed to : \n",(char*)curr->data);
		curr = curr->next;
	}
}

void printMsgs(Node* head){
	Node* curr = head;
	printf("Msgs on Memory : %d total\n", LinkedList_getSize(head) );
	
	while(curr != NULL){
		//TODO
		printf("\t%s ,  : \n",(char*)curr->data);
		curr = curr->next;
	}
}



void shutdown(int signal){
	printf("Exiting\n");
	unlink(LISTENERPATH);
	Node* curr = usersHead;
	while(curr != NULL){
		//TODO
		curr = curr->next;
	}
	exit(0);
}


void accquireLock(){
}


void* clientMessageReciever(void* data){
	
	int result = mkfifo(LISTENERPATH,0666);
	if(result != 0) {
		printf("[ERROR]Creating listener fifo");
	}
	int fifo = open(LISTENERPATH,O_RDWR);
	if(fifo == -1 ){
		printf("[ERROR]Main client listener creation error (FIFO)\n");
		exit(0);
	}

	
	while(1){
		int const bufferSize = 2048;
		char buff[bufferSize];
		char* buffer = buff;

		int bCount = read(fifo, buffer, bufferSize * sizeof(char));
		printf("[INFO]Recebeu bytes : %d\n",bCount);
		Command* command = (Command*)buffer;
		buffer = buffer + sizeof(Command);


		printf("[INFO]Recebeu commando : %d \n",command->cmd);

		if(command->cmd == NEW_USER){
			NewClientInfo* info = (NewClientInfo*)buffer;
			User* user = malloc(sizeof(User));
			user->pid = info->pid;
			strcpy(user->username,info->username);
			printf("[INFO]Client fifo : %s\n",info->pathToFifo);

			FILE* file = fopen(info->pathToFifo,"w");
			if(file == NULL){
				printf("[ERROR]Error opening user fifo\n");
			} else{
				printf("[INFO]Opened client fifo\n");
			}
			//TODO adicionar o user á lista


		}else if(command->cmd == NEW_MESSAGE){

		}

		/*NewClientInfo * info = malloc(sizeof(NewClientInfo)); ;
		char* piece = strtok(buffer,",");
		info->id = atoi(piece);
		piece = strtok(buffer,",");
		strcpy(info->pathFIFO,piece);*/
	}

}
