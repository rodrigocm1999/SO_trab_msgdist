#include "cliente.h"

ClientConfig cfg;

int main(int argc,char* argv[]){
	if(!isServerRunning()){
		printf("Server not running\nExiting\n");
		exit(0);
	}
	
	printf("Username: \n");
	scanf("%s",cfg.username);

	//Criar fifo para servidor enviar coisas para cliente
	// Enviar PID, username e talvez path para FIFO
	// criar fifo em /tmp/
	sprintf (cfg.fifoPath,"/tmp/%d",getpid());
	mkfifo(cfg.fifoPath,0666);
	cfg.fifo = open(cfg.fifoPath,O_RDWR);
	printf("Sarting \n");
	if(cfg.fifo == -1 ){
		printf("fifo creation error (FIFO)\n");
		printf("Error: %d\n",errno);
		exit(0);
	}

	// printf("ola");
	cfg.server = open(LISTENERPATH,O_WRONLY);
	{
		NewClientInfo newClient;
		strcpy(newClient.username,cfg.username);
		newClient.pid = getpid();
		strcpy(newClient.pathToFifo,cfg.fifoPath);

		Command command;
		command.cmd = NEW_USER;
		strcpy(command.username,cfg.username);
		
		Buffer buffer = joinCommandStruct(&command,&newClient,sizeof(NewClientInfo));
		
		int bWriten = write(cfg.server,buffer.ptr,buffer.size);
		free(buffer.ptr);
		printf("Command: %d\tSize: %d\tBytes Written: %d\n",command.cmd,buffer.size,bWriten);
	}
	// Criar thread para receber info do servidor
	pthread_t listenerThread; 
	pthread_create(&listenerThread,NULL,fifoListener,(void*)NULL);

	int choice;
	while(1){
		
		printf("\n1 - Write Message\n2 - List Topics\n3 - List Titles in Topic\n4 - Read Message in Topic\n5 - Subscribe to Topic\n6 - Unsubscribe from Topic\n0 - Exit\n");
		scanf("%d",&choice);

		switch (choice)
		{
		case 0: // Exit
			printf("Exiting\n");
			exit(0);
			break;
		case 1: 
				printf("Ola");
				char topic[20], titulo[100];
				char msg[1000];


				printf("Topico da menssagem: ");
				scanf("%s",topic);


				printf("Titulo da menssagem: ");
				scanf("%s",titulo);


				printf("Mensagem: ");
				scanf("%s",msg);

				Message newMsg(username,topic,titulo,msg);


				printf("Mensagem enviada");

			break;

		case 2: // List Topics
			break;

		case 3: // List Titles in Topic
			break;

		case 4: // Read Message in Topic
			break;

		case 5: // Subscribe to Topic
			break;

		case 6:// Unsubscribe to topic
			break;

		default:
			printf("Invalid Option\n");
			break;
		}
		

	}


	/*initscr();
	cbreak();
	int nLines = 20,nCols = 100,y0 = 10,x0 = 10;
	WINDOW * win = newwin(nLines, nCols, y0, x0);
	wrefresh(win);
	while(1){pause();}*/
	return 0;
}


void* fifoListener(void* data){
	


}
/*
void newMessage(char username){
	char topic[20], titulo[100];
	char msg[1000];


	printf("Topico da menssagem: ");
	scanf("%s",topic);


	printf("Titulo da menssagem: ");
	scanf("%s",titulo);


	printf("Mensagem: ");
	scanf("%s",msg);

	Message newMsg(username,topic,titulo,msg);


	printf("Mensagem enviada");

}

void ListTopics(	){
	
}*/