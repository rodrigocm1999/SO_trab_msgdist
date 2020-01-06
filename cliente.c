#include "cliente.h"

ClientConfig cfg;

int main(int argc, char *argv[])
{

	// Checks if server is running
	if (!isServerRunning())
	{
		printf("Server not running\nExiting\n");
		exit(0);
	}

	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);

	if (argc == 2)
	{
		strcpy(cfg.username, argv[1]);
	}
	else
	{
		printf("Username: \n");
		scanf("%s", cfg.username);
	}
	//Criar fifo para servidor enviar coisas para cliente
	// Enviar PID, username e talvez path para FIFO
	// criar fifo em /tmp/
	{
		sprintf(cfg.fifoPath, "/tmp/%d", getpid());
		int result = mkfifo(cfg.fifoPath, 0666);
		if (result != 0)
		{
			fprintf(stderr, "[ERROR]Creating listener fifo");
		}

		cfg.fifo = open(cfg.fifoPath, O_RDWR);
		printf("Starting \n");
		if (cfg.fifo == -1)
		{
			printf("fifo creation error (FIFO)\n");
			printf("Error: %d\n", errno);
			exit(0);
		}
	}
	//Connect to fifo and send new user to server
	{
		pthread_t thread;
		pthread_create(&thread, NULL, checkServerRunning, NULL);
		cfg.server_running = false;
		cfg.server = open(LISTENER_PATH, O_WRONLY);
		cfg.server_running = true;
		NewClientInfo newClient;
		strcpy(newClient.username, cfg.username);
		newClient.pid = getpid();
		strcpy(newClient.pathToFifo, cfg.fifoPath);

		sendToServer(NEW_USER, &newClient, sizeof(NewClientInfo));

		char buff[1024];
		void *buffer = buff;

		int bCount = read(cfg.fifo, buffer, 1024);
		Command *command = (Command *)buffer;
		buffer = buffer + sizeof(Command);

		if (command->cmd == USERNAME_REPEATED)
		{
			char *newUsername = (char *)buffer;
			strcpy(cfg.username, newUsername);
			printf("Username was repeated, changed to : %s\n", newUsername);
			sleep(1);
		}
	}
	// Criar thread para receber info do servidor
	pthread_t listenerThread;
	pthread_create(&listenerThread, NULL, fifoListener, (void *)NULL);
	pthread_t heartbetThread;
	pthread_create(&listenerThread, NULL, heartbeat, (void *)NULL);

	int menu_ret = 1, menu_ret2 = 1;
	char alts[][100] = {
		 {"Write Message"}, 
		 {"Info needed"},
		 {"Subscribe to Topic"},
		 {"Unsubscribe from Topic"}, 
		 {"Exit"},
	}; 
	char alts2[][100] = {
		 {"List Topics"},
		 {"List Titles in Topic"},
		 {"Read Message in Topic"},
		 {"Return"},
	};

	setlocale(LC_CTYPE, "");

	initscr();					
	noecho();					
	keypad(stdscr, true);	
	meta(stdscr, true);		
	nodelay(stdscr, false); 
	notimeout(stdscr, true);
	raw();
	curs_set(0);

	int choice;

	while (menu_ret != 5)
	{

		menu_ret = print_menu(0, 0, 5, 15,
									 cfg.username, alts, menu_ret);

		if (menu_ret == 1) 
		{
			erase();
			refresh();
			WINDOW *newMessageWindow = newwin(20-2, 70-2, 2, 2);
			//box(newMessageWindow, 0, 0);

			WINDOW *border_window =   newwin(20, 70, 1, 1);
			box(border_window,0,0);
			scrollok(newMessageWindow,true);
			wrefresh(border_window);
			wrefresh(newMessageWindow);

			Message message;
			strcpy(message.username, cfg.username);

			echo();
			mvwprintw(newMessageWindow, 0, 1, "Topic: ");
			wgetstr(newMessageWindow, message.topic);

			mvwprintw(newMessageWindow, 1, 1, "Title: ");
			refresh();
			wgetstr(newMessageWindow, message.title);

			mvwprintw(newMessageWindow, 3, 1, "Body: ");
			refresh();
			wgetstr(newMessageWindow, message.body);

			mvwprintw(newMessageWindow, 17, 1, "Message Duration: ");
			refresh();
			char temp[20];
			wscanw(newMessageWindow, "%s", temp);
			message.duration = atoi(temp);

			sendToServer(NEW_MESSAGE, &message, sizeof(Message));
			mvwprintw(newMessageWindow, 17, 53, "Message sent");
			wrefresh(newMessageWindow);
			getch();
		}
		if (menu_ret == 2) 
		{
			erase();						 
			do					
			{
				menu_ret2 = print_menu(0, 0, 4, 15, "SELECT", alts2, 1);

				if (menu_ret2 == 1)
				{
					// List Topics
					erase();
					refresh();

					cfg.win.topicsWindow = newwin(20, 70, 1, 1);
					box(cfg.win.topicsWindow, 0, 0);
					wrefresh(cfg.win.topicsWindow);

					sendToServer(GET_TOPICS, NULL, 0);

					getch();
					erase();
					refresh();
				}
				if (menu_ret2 == 2)
				{
					erase();
					refresh();

					cfg.win.titlesFromTopicWindow = newwin(20, 70, 1, 1);
					box(cfg.win.titlesFromTopicWindow, 0, 0);
					wrefresh(cfg.win.titlesFromTopicWindow);

					char topic[TOPIC_L];
					echo();
					mvwprintw(cfg.win.titlesFromTopicWindow, 1, 1, "Topic: ");
					wrefresh(cfg.win.titlesFromTopicWindow);
					wgetstr(cfg.win.titlesFromTopicWindow, topic);

					sendToServer(LIST_TOPIC_MESSAGES, topic, sizeof(topic));

					getch();
					erase();
					refresh();
					// List Titles in Topic
				}
				if (menu_ret2 == 3)
				{
					erase();
					refresh();

					cfg.win.messageFromTopicWindow = newwin(20, 70, 1, 1);
					box(cfg.win.messageFromTopicWindow, 0, 0);
					wrefresh(cfg.win.messageFromTopicWindow);

					echo();
					mvwprintw(cfg.win.messageFromTopicWindow, 1, 1, "Topic ID: ");
					int id;
					wrefresh(cfg.win.messageFromTopicWindow);
					wscanw(cfg.win.messageFromTopicWindow, "%d", &id);
					sendToServer(GET_MESSAGE, &id, sizeof(int));

					getch();
					erase();
					refresh();
				}
				refresh();
			} while (menu_ret2 != 4);
		}
		if (menu_ret == 3)
		{
			//Subscribe to Topic
			char buffer[TOPIC_L];

			mvprintw(7, 26, "Topic Name: ");
			refresh();
			getstr(buffer);

			sendToServer(SUBSCRIBE_TOPIC, buffer, TOPIC_L);
		}
		if (menu_ret == 4)
		{
			//Unsubscribe to Topic
			char buffer[TOPIC_L];

			mvprintw(7, 26, "Topic Name: ");
			refresh();
			getstr(buffer);

			sendToServer(UNSUBSCRIBE_TOPIC, buffer, TOPIC_L);
		}

		if (menu_ret == 5)
		{
			//mvprintw(6, 34, "Exiting");
			shutdown(true);
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

void *checkServerRunning(void *data)
{
	sleep(1);
	if (cfg.server_running == false)
	{
		printf("Server not running\n");
		shutdown(false);
	}
}

void *fifoListener(void *data)
{
	while (1)
	{
		int const bufferSize = 8192;
		char buff[bufferSize];
		void *buffer = buff;

		int bCount = read(cfg.fifo, buffer, bufferSize);
		//fprintf(stderr, "[INFO]Recived bytes : %d\n", bCount);
		Command *command = (Command *)buffer;
		buffer = buffer + sizeof(Command);

		switch (command->cmd)
		{

		case SERVER_SHUTDOWN:
		{
			mvprintw(17, 5, "Server Shuting Down"); // Intented to not change line
			refresh();
			shutdown(false);
		}
		case KICKED:
		{
			mvprintw(17, 5, "Kicked from server\nServer "); // Intented to not change line
			refresh();
			shutdown(false);
		}
		case SUBSCRIBED_TO_TOPIC:
		{
			mvprintw(17, 5, "Subscrition sucessuful\n");
			refresh();
			break;
		}
		case UNSUBSCRIBE_TOPIC:
		{
			mvprintw(17, 5, "Unsubscrition sucessuful\n");
			refresh();
			break;
		}
		case TOPIC_DELETED:
		{
			mvprintw(17, 5, "Topic deleted\n");
			refresh();
			break;
		}
		case ALREADY_SUBSCRIBED:
		{
			mvprintw(17, 5, "Already subscribed\n");
			refresh();
			break;
		}
		case NON_EXISTENT_TOPIC:
		{
			mvprintw(17, 5, "Non existent topic\n");
			refresh();
			break;
		}
		case MESSAGE_NOTIFICATION:
		{
			MessageNotification *notification = buffer;
			sleep(1);
			mvprintw(17, 5, "New Message with id : '%d' from topic : '%s'", notification->id, notification->topic);
			refresh();
			break;
		}
		case BAD_MESSAGE:
		{
			mvprintw(17, 5, "Message discarded\n");
			refresh();
			break;
		}
		case GET_TOPICS:
		{
			int *n_topicos = buffer;
			char *topicos;
			buffer = buffer + sizeof(int);

			topicos = buffer;
			mvwprintw(cfg.win.topicsWindow, 1, 1, "Topics:");
			wrefresh(cfg.win.topicsWindow);
			for (int i = 0; i < *n_topicos; i++)
				mvwprintw(cfg.win.topicsWindow, 3 + i, 1, "- %s", &topicos[i * TOPIC_L]);

			wrefresh(cfg.win.topicsWindow);
			break;
		}
		case LIST_TOPIC_MESSAGES:
		{

			int *n_titles = buffer;
			buffer = buffer + sizeof(int);
			if (*n_titles != 0)
			{
				for (int i = 0; i < *n_titles; i++)
				{
					MessageInfo *info = buffer + i * sizeof(MessageInfo);
					mvwprintw(cfg.win.titlesFromTopicWindow, 3 + i, 1, "- ID: %d, Title: %s", info->id, info->title);
				}
			}
			else
				mvwprintw(cfg.win.titlesFromTopicWindow, 5, 5, "Topic without menssages!");

			wrefresh(cfg.win.titlesFromTopicWindow);

			break;
		}
		case GET_MESSAGE:
		{

			Message *info = buffer;
			mvwprintw(cfg.win.messageFromTopicWindow, 3, 1, "Title: %s", info->title);
			mvwprintw(cfg.win.messageFromTopicWindow, 4, 1, "Body:");
			mvwprintw(cfg.win.messageFromTopicWindow, 5, 1, "%s", info->body);
			wrefresh(cfg.win.messageFromTopicWindow);

			break;
		}
		case MESSAGE_NOT_FOUND:
		{
			mvwprintw(cfg.win.messageFromTopicWindow, 8, 8, "Message does not exist!");
			wrefresh(cfg.win.messageFromTopicWindow);
			break;
		}
		default:
			mvprintw(17, 5, "Not Recognized Command\n");
			refresh();
			break;
		}
	}
}

void shutdown(int warnServer)
{
	mvprintw(17, 5, "Shuting Down\n");
	refresh();
	if (warnServer)
	{
		sendToServer(USER_LEAVING, NULL, 0);
	}
	close(cfg.server);
	close(cfg.fifo);
	unlink(cfg.fifoPath);
	sleep(1);
	endwin();
	exit(0);
}

void signalHandler(int signal)
{
	if (signal == SIGPIPE)
	{
		printf("Server Down\n");
	}
	shutdown(false);
}

void *heartbeat(void *data)
{
	while (1)
	{
		sleep(9);
		sendToServer(HEARTBEAT_ISALIVE, NULL, 0);
	}
}

int sendToServer(int cmd, void *other, size_t size)
{
	Command command;
	command.cmd = cmd;
	command.senderPid = getpid();
	command.structSize = size;

	Buffer buffer = joinCommandStruct(&command, other, size);
	int written = write(cfg.server, buffer.ptr, buffer.size);
	free(buffer.ptr);
	return written;
}
