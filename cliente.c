#include "cliente.h"


ClientConfig cfg;

int main(int argc,char* argv[]){

	// Checks if server is running
	if(!isServerRunning()){
		printf("Server not running\nExiting\n");
		exit(0);
	}
	
	signal(SIGINT,shutdown);
	signal(SIGPIPE,shutdown);
	printf("%d\n",getpid());
	printf("Username: \n");
	scanf("%s",cfg.username);

	//Criar fifo para servidor enviar coisas para cliente
	// Enviar PID, username e talvez path para FIFO
	// criar fifo em /tmp/
	{
		sprintf (cfg.fifoPath,"/tmp/%d",getpid());
		int result = mkfifo(cfg.fifoPath,0666);
		if(result != 0) {
			fprintf(stderr,"[ERROR]Creating listener fifo");
		}

		cfg.fifo = open(cfg.fifoPath,O_RDWR);
		printf("Starting \n");
		if(cfg.fifo == -1 ){
			printf("fifo creation error (FIFO)\n");
			printf("Error: %d\n",errno);
			exit(0);
		}
	}

	cfg.server = open(LISTENER_PATH,O_WRONLY);
	{
		NewClientInfo newClient;
		strcpy(newClient.username,cfg.username);
		newClient.pid = getpid();
		strcpy(newClient.pathToFifo,cfg.fifoPath);

		sendToServer(NEW_USER,&newClient,sizeof(NewClientInfo));


		char buff[1024];
		void* buffer = buff;

		int bCount = read(cfg.fifo, buffer, 1024);
		Command* command = (Command*)buffer;
		buffer = buffer + sizeof(Command);

		if(command->cmd == USERNAME_REPEATED){
			char* newUsername = (char*)buffer;
			strcpy(cfg.username,newUsername);
			printf("Username was repeated, changed to : %s\n", newUsername);
			sleep(1);
		}
		
	}
	// Criar thread para receber info do servidor
	pthread_t listenerThread;
	pthread_create(&listenerThread,NULL,fifoListener,(void*)NULL);
	pthread_t heartbetThread;
	pthread_create(&listenerThread,NULL,heartbeat,(void*)NULL);

	int menu_ret = 1, menu_ret2 = 1;
    char alts[][100] = {{" Write Message"},                 /* Every menu needs an */
                         {"Info needed"},
                         {"Subscribe to Topic"}, 
                         {"Unsubscribe from Topic"},        /* array like these to */
                         {"Exit"},};                  /* hold the entries.   */
    char alts2[][100] = {{"List Topics"},
                         {"List Titles in Topic"},
                         {"Read Message in Topic"},
                         {"Return"},};

    setlocale (LC_CTYPE, "");

    initscr();                  /* Most of the below initialisers are */
    noecho();                   /* not necessary for this example.    */
    keypad (stdscr, TRUE);      /* It's just a template for a         */
    meta (stdscr, TRUE);        /* hypothetical program that might    */
    nodelay (stdscr, FALSE);    /* need them.                         */
    notimeout (stdscr, TRUE);
    raw();
    curs_set (0);

	int choice;

	while(menu_ret != 5){
		
		menu_ret = print_menu (2, 5, 5, 15,
                               cfg.username, alts, menu_ret);

		if (menu_ret == 1)  /* This is just an example program. */
        {                   /* You can't start an actual game.  */

			Message message;
			strcpy(message.username,cfg.username);
			char topic[20], titulo[100];
			char msg[1000];

			echo();
			mvprintw(6, 27,"Topico da menssagem: ");
			refresh();
			scanw("%s",message.topic);
			


			mvprintw(8, 27,"Titulo da menssagem: ");
			refresh();
			scanw("%s",message.title);

			//Message newMsg(username,topic,titulo,msg);

			mvprintw(10,27,"Mensagem: ");
			refresh();
			scanw("%s",message.body);

			sendToServer(NEW_MESSAGE,&message,sizeof(Message));
			mvprintw(12,27,"Mensagem enviada\n");
            getch();
        }
		if (menu_ret == 2) /* If you select load game, a new    */
        {                       /* menu will show up without erasing */
            do                  /* the main menu. */
            {
                menu_ret2 = print_menu (6, 34, 4, 15,
                                        "SELECT SLOT", alts2, 1);


				if (menu_ret2 == 1 ){
					// List Topics
					sendToServer(GET_TOPICS,NULL,0);
					
				}
				if (menu_ret2 == 2 ){
					
					// List Titles in Topic

				}
				if (menu_ret2 == 3 ){
					
					// Read Message in Topic

				}
            }
            while (menu_ret2 != 4);
        }
		if(menu_ret == 3){
			//Subscribe to Topic
			char buffer[TOPIC_L];

			mvprintw(10,27,"Topic Name: ");
			refresh();
			scanw("%s",buffer);

			sendToServer(SUBSCRIBE_TOPIC,buffer,TOPIC_L);
		}
		if(menu_ret == 4){
			//Unsubscribe to Topic
			char buffer[TOPIC_L];

			mvprintw(10,27,"Topic Name: ");
			refresh();
			scanw("%s",buffer);

			sendToServer(UNSUBSCRIBE_TOPIC,buffer,TOPIC_L);
		}

		if(menu_ret == 5){
			mvprintw(6, 34,"Exiting");
			shutdown(SIGINT);
		}

		erase(); 
		// Write Message
		// List Topics
		// List Titles in Topic
		// Read Message in Topic
		// Subscribe to Topic
		// Unsubscribe to topic

	}

	endwin();
	return 0;
}


void* fifoListener(void* data){
	while(1){
		int const bufferSize = 8192;
		char buff[bufferSize];
		void* buffer = buff;

		int bCount = read(cfg.fifo, buffer, bufferSize);
		fprintf(stderr,"[INFO]Recebeu bytes : %d\n",bCount);
		Command* command = (Command*)buffer;
		buffer = buffer + sizeof(Command);


		switch (command->cmd) {

			case SERVER_SHUTDOWN:{
				printf("Server "); // Intented to not change line
				shutdown(SIGINT);
			}
			case SUBSCRIBED_TO_TOPIC:{
				printf("Subscrition sucessuful\n");
				break;
			}
			case UNSUBSCRIBE_TOPIC:{
				printf("Unsubscrition sucessuful\n");
				break;
			}
			case NON_EXISTENT_TOPIC:{
				printf("Non existent topic\n");
				break;
			}

			
			default:
				printf("Not Recognized Command\n");
				break;
		}
	}
}

void shutdown(int signal){
	if(signal == SIGPIPE){
		printf("Server Down\n");
	}
	printf("Shuting Down\n");
	sendToServer(USER_LEAVING,NULL,0);

	close(cfg.server);
	close(cfg.fifo);
	unlink(cfg.fifoPath);
	execlp("reset","reset",NULL); // resets terminal
	exit(0);
}

void* heartbeat(void* data){
	while(1){
		sleep(9);
		sendToServer(HEARTBEAT_ISALIVE,NULL,0);
	}
}

int sendToServer(int cmd,void* other,size_t size){
	Command command;
	command.cmd = cmd;
	command.senderPid = getpid();
	command.structSize = size;
	
	Buffer buffer = joinCommandStruct(&command,other,size);
	int written = write(cfg.server,buffer.ptr,buffer.size);
	free(buffer.ptr);
	return written;
}