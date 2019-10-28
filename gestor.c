#include "gestor.h"

void* waitForNewClients(void* data){
	//char mainFIFO[] = "./fifos/listener";
	char mainFIFO[] = "/tmp/gestorListenerFifo";
	int fifo = open(mainFIFO,O_RDONLY);
	if(fifo == -1){
		mkfifo(mainFIFO,0666);
		fifo = open(mainFIFO,O_RDONLY);
		if(fifo == -1 ){
			printf("Main client listener creation error (FIFO)\n");
			printf("Error: %d\n",errno);
			exit(0);
		}
	}
	int const bufferSize = 128;
	char buffer[bufferSize];
	while(1){
		read(fifo, buffer, bufferSize * sizeof(char));
		
		NewClientInfo * info = malloc(sizeof(NewClientInfo)); ;
		char* piece = strtok(buffer,",");
		info->id = atoi(piece);
		piece = strtok(buffer,",");
		strcpy(info->pathFIFO,piece);

		pthread_t thread;
		pthread_create(&thread,NULL,handleClient,(void*)info);
	}

}

void shutdown(int signal){
	printf("Exiting\n");
	exit(0);
}

int main(int argc,char* argv[]){

	//check if process already running
	if (isServerRunning()){
		printf("Program already running\nExiting\n");
		exit(0);
	}

	//start verificador
	int verifPipe[2];
	pipe(verifPipe);
	int reciveVerifPipe[2];
	pipe(reciveVerifPipe);
	
	int childPid = fork();
	if(childPid == 0){
		close(0);
		dup(verifPipe[0]);
		close(verifPipe[0]);
		close(verifPipe[1]);


		close(1);
		dup(reciveVerifPipe[1]);
		close(reciveVerifPipe[0]);
		close(reciveVerifPipe[1]);

		//start verifier on child process
		char badWordsFile[] = "badwords.txt";
		char ver[] = "verificador";
		execl(ver, ver, badWordsFile, (char*)NULL);
		printf("Bad Word Verifier Not started.");
		exit(0);
		//Exec gave error
	}else
	{
		close(verifPipe[0]);
		close(reciveVerifPipe[1]);
	}
	

	

	signal(SIGINT, shutdown);

	pthread_t listenerThread; // Listener for new Clients
	pthread_create(&listenerThread,NULL,waitForNewClients,(void*)NULL);


	// Create lists for memory objects
	Node* msgsHead = new_Node(NULL);
	Node* topicsHead = new_Node(NULL);
	Node* usersHead = new_Node(NULL);
	int filter = 0;
	/*List* msgsHead = new_List();
	List* topicsHead = new_List();
	List* usersHead = new_List();*/

	printf("Started\n");

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
			int const bufferSize = 1024;
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

void verifyBadWords(){
	char msgEnd[] = "##MSGEND##";
}

Message* new_Message(){
	Message* obj = malloc(sizeof(Message));
	obj->id = ++msgId;

	return obj;
}

void* handleClient(void* data){
	NewClientInfo * info = data;
	


	//Dont forget to free it
	free(info);
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
