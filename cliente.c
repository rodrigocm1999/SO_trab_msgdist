#include "cliente.h"

ClientConfig cfg;

int main(int argc,char* argv[]){
	if(!isServerRunning()){
		printf("Server not running\nExiting\n");
		exit(0);
	}
	

	printf("%d\n",getpid());
	printf("Username: \n");
	scanf("%s",cfg.username);

	//Criar fifo para servidor enviar coisas para cliente
	// Enviar PID, username e talvez path para FIFO
	// criar fifo em /tmp/
	sprintf (cfg.fifoPath,"/tmp/%d",getpid());
	int result = mkfifo(cfg.fifoPath,0666);
	if(result != 0) {
		fprintf(stderr,"[ERROR]Creating listener fifo");
	}

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

		sendToServer(NEW_USER,&newClient,sizeof(NewClientInfo));
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
			shutdown();
			break;
		case 1:

			printf("Ola\n");
			Message message;
			char topic[20], titulo[100];
			char msg[1000];


			printf("Topico da menssagem: ");
			scanf("%s",message.topic);


			printf("Titulo da menssagem: ");
			scanf("%s",message.title);


			printf("Mensagem: ");
			scanf("%s",message.body);


			printf("Mensagem enviada\n");

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
	while(1){
		int const bufferSize = 2048;
		char buff[bufferSize];
		char* buffer = buff;

		int bCount = read(cfg.server, buffer, bufferSize * sizeof(char));
		fprintf(stderr,"[INFO]Recebeu bytes : %d\n",bCount);
		Command* command = (Command*)buffer;
		buffer = buffer + sizeof(Command);

		if(command->cmd == SERVER_SHUTDOWN){
			shutdown();
		}
		

	}
}

void shutdown(){
	printf("Shuting Down\n");
	close(cfg.server);
	close(cfg.fifo);
	unlink(cfg.fifoPath);
	exit(0);
}


void ListTopics(){
	
}

int sendToServer(int cmd,void* other,size_t size){
	Command command;
	command.cmd = cmd;
	command.clientPid = getpid();
	command.structSize = size;
	strcpy(command.username,cfg.username);
	Buffer buffer = joinCommandStruct(&command,other,size);
	int written = write(cfg.server,buffer.ptr,buffer.size);
	free(buffer.ptr);
	return written;
}