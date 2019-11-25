#include "gestor.h"

ServerConfig cfg;

int main(int argc, char *argv[])
{
	// Variable Initialization
	{
		cfg.msgId = 0;
		cfg.filter = 1;
		cfg.users.head = NULL;
		cfg.topics.head = NULL;
		cfg.msgs.head = NULL;
		int error = 0;
		error |= pthread_mutex_init(&cfg.mutex.msgsLock, NULL);
		error |= pthread_mutex_init(&cfg.mutex.topicsLock, NULL);
		error |= pthread_mutex_init(&cfg.mutex.usersLock, NULL);
		if (error != 0)
		{
			printf("Mutex creation has failed\n");
			shutdown(SIGINT);
		}
	}
	//check for arguments and already running server
	{
		int checkServerRunning = 1;
		int res;
		while ((res = getopt(argc, argv, "f")) != -1)
		{
			switch (res)
			{
			case 'f':
				printf("[INFO] Using force start option\n");
				if (isServerRunning())
				{
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
		if (checkServerRunning && isServerRunning())
		{
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

		char *badWordsFile = getenv(WORDSNOT);
		if (badWordsFile == NULL)
		{
			badWordsFile = DEFAULTWORDSNOT;
		}
		printf("[INFO] WORDSNOT = %s\n", badWordsFile);

		int childPid = fork();
		if (childPid == 0)
		{
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
			execl(ver, ver, badWordsFile, (char *)NULL);
			fprintf(stderr, "[ERROR] Bad Word Verifier Not started.\n");
			exit(0);
			//Exec gave error
		}
		else
		{
			close(sendPipe[0]);
			close(recievePipe[1]);
		}
		cfg.sendVerif = sendPipe[1];
		cfg.recieveVerif = recievePipe[0];
	}

	//Max bad Words
	{
		char *temp = getenv(MAXNOT);
		if (temp != NULL)
		{
			cfg.maxbadWords = atoi(temp);
		}
		else
		{
			cfg.maxbadWords = DEFAULTMAXNOT;
		}
		printf("[INFO] MAXNOT = %d\n", cfg.maxbadWords);
	}

	// Start listener for messages
	{
		pthread_t listenerThread;
		pthread_create(&listenerThread, NULL, clientMessageReciever, (void *)NULL);
		pthread_t checkClientsThread;
		pthread_create(&listenerThread, NULL, checkAllClientsState, (void *)NULL);
		pthread_t checkMessageTimeoutThread;
		pthread_create(&listenerThread, NULL, checkMessageTimeout, (void *)NULL);
	}

	// Change Default Signal Effect
	signal(SIGINT, shutdown);

	printf("Write \"help\" to get command information\n");

	char command[512];
	char *cmd;
	while (1)
	{
		printf("-> ");
		fgets(command, 512, stdin);
		cmd = strtok(command, DELIM);
		if (cmd != NULL)
		{

			if (strcmp(cmd, "filter") == 0)
			{
				char *token = strtok(NULL, DELIM);
				if (token != NULL)
				{
					if (strcmp(token, "on") == 0)
					{
						cfg.filter = 1;
						printf("Filter is on\n");
					}
					else if (strcmp(token, "off") == 0)
					{
						cfg.filter = 0;
						printf("Filter is off\n");
					}
					else if (strcmp(token, "status") == 0)
					{
						if (cfg.filter == 0)
							printf("Filter is off\n");
						else
							printf("Filter is on\n");
					}
				}
				else
				{
					printf("\nInvalid filter option, available : on off status\n");
				}
			}

			else if (strcmp(cmd, "users") == 0)
			{
				lock_topics(true);
				lock_users(true);
				printUsers(cfg.users.head);
				lock_topics(false);
				lock_users(false);
			}

			else if (strcmp(cmd, "topics") == 0)
			{
				lock_topics(true);
				lock_users(true);
				printTopics(cfg.topics.head);
				lock_topics(false);
				lock_users(false);
			}

			else if (strcmp(cmd, "msg") == 0)
			{
				lock_msgs(true);
				char *msg_id = strtok(NULL, DELIM);
				if (msg_id == NULL)
					printMsgs(cfg.msgs.head);
				else
				{
					int id = atoi(msg_id);
					Message *msg = getMessageById(id);
					if (msg == NULL)
						printf("No message with id : %s\n", msg_id);
					else
						printf("Title : %s,\nUsername : %s,\nTopic : %s,\nBody : %s;\n", msg->title, msg->username, msg->topic, msg->body);
				}
				lock_msgs(false);
			}

			else if (strcmp(cmd, "topic") == 0)
			{
				lock_topics(true);
				lock_msgs(true);
				char *topic = strtok(NULL, DELIM);
				if (topic != NULL)
				{
					if (getTopicNode(topic) == NULL)
					{
						printf("Non existing topic : '%s'\n", topic);
					}
					else
					{
						int counter = 0;
						Node *curr = cfg.msgs.head;
						while (curr != NULL)
						{
							Message *currMessage = (Message *)curr->data;
							if (strcmp(currMessage->topic, topic) == 0)
							{
								++counter;
								printf("Id : %d, Title : %s\n", currMessage->id, currMessage->title);
							}
							curr = curr->next;
						}
						printf("Total %d\n", counter);
					}
				}
				else
				{
					printf("Invalid command usage\nExample : topic TOPICNAME\n");
				}
				lock_topics(false);
				lock_msgs(false);
			}

			else if (strcmp(cmd, "del") == 0)
			{
				lock_msgs(true);
				char *token = strtok(NULL, DELIM);
				int id = atoi(token);
				if (id == 0)
				{
					printf("Invalid Identifier\n");
				}
				else
				{
					int found = false;
					Node *curr = cfg.msgs.head;
					while (curr != NULL)
					{
						Message *message = (Message *)curr->data;
						if (message->id == id)
						{
							LinkedList_detachNode(&cfg.msgs, curr);
							free(message);
							free(curr);
							printf("Deleted , id = \"%d\"\n", id);
							found = true;
							break;
						}
						curr = curr->next;
					}
					if (found == false)
					{
						printf("Message with id = \"%d\" not found\n", id);
					}
				}
				lock_msgs(false);
			}

			else if (strcmp(cmd, "kick") == 0)
			{
				lock_users(true);
				char *username = strtok(NULL, DELIM);
				if (username != NULL)
				{
					Node *userNode = getUserNodeByUsername(username);

					if (userNode != NULL)
					{
						sendToClient((User *)userNode->data, KICKED, NULL, 0);
						userLeft(userNode);
					}
					else
					{
						printf("No user with name : '%s'\n", username);
					}
				}
				else
				{
					printf("Invalid command usage\nExample : kick USERNAME\n");
				}
				lock_users(false);
			}

			else if (strcmp(cmd, "shutdown") == 0)
			{
				lock_users(true);
				shutdown(SIGINT);
			}

			else if (strcmp(cmd, "prune") == 0)
			{
				lock_all(true);
				// Remove topics without messages and unsubscibes users
				Node *topic_node = cfg.topics.head;
				int topics_removed = 0;
				LinkedList list;
				list.head = NULL;
				while (topic_node != NULL)
				{
					char *topic_to_remove = (char *)topic_node->data;
					//check if has messages
					if (messagesInTopic(topic_to_remove) == 0)
					{
						++topics_removed;
						//remove topic and unsubscribe users
						Node *user_node = cfg.users.head;
						while (user_node != NULL)
						{
							User *user = (User *)user_node->data;
							deleteUserTopic(user, topic_to_remove); // only takes out topic if it is there
							user_node = user_node->next;
						}

						LinkedList_detachNode(&cfg.topics, topic_node);
						LinkedList_appendNode(&list, topic_node);
					}

					topic_node = topic_node->next;
				}

				printf("Removed topics :");
				Node *node = list.head;
				while (node != NULL)
				{
					printf(" %s", (char *)node->data);
					Node *node_to_delete = node;
					node = node->next;
					// remove topic from memory
					free(node_to_delete->data);
					free(node_to_delete->previous);
				}
				printf("\n");
				lock_all(false);
			}

			else if (strcmp(cmd, "help") == 0)
			{
				FILE *file = fopen("help.txt", "r");
				int const bufferSize = 2048;
				char buffer[bufferSize];
				int sdas = fread(buffer, sizeof(char), bufferSize, file);
				printf("\n");
				write(0, buffer, strlen(buffer));
				printf("\n\n");
			}

			else if (strcmp(cmd, "verify") == 0)
			{
				char *token = strtok(NULL, DELIM);
				while (token != NULL)
				{
					write(cfg.sendVerif, token, strlen(token));
					write(cfg.sendVerif, "\n", 1);
					token = strtok(NULL, DELIM);
				}
				write(cfg.sendVerif, MSGEND, MSGEND_L);
				char buffer[4];
				read(cfg.recieveVerif, buffer, 4);
				int nBadWords = atoi(buffer);
				printf("Number of bad words : %d\n", nBadWords);
			}

			else
			{
				printf("Write \"help\" to get command information\n");
			}
		}
	}
	return 0;
}

// TODO
void *clientMessageReciever(void *data)
{

	int result = mkfifo(LISTENER_PATH, 0666);
	if (result != 0)
	{
		fprintf(stderr, "[ERROR]Creating listener fifo : already exists\n");
		exit(0);
	}
	int fifo = open(LISTENER_PATH, O_RDWR);
	if (fifo == -1)
	{
		printf("[ERROR]Main client listener creation error (FIFO)\n");
		fprintf(stderr, "[ERROR]Main client listener creation error (FIFO)\n");
		exit(0);
	}

	while (1)
	{
		char buff[RECIEVE_BUFFER_SIZE];
		void *buffer = buff;

		int bCount = read(fifo, buffer, RECIEVE_BUFFER_SIZE * sizeof(char));
		Command *command = (Command *)buffer;
		buffer = buffer + sizeof(Command);
		//fprintf(stderr,"[INFO]Recebeu commando : %d , size : %zu , clientPid : %d\n", command->cmd,command->structSize,command->clientPid);

		switch (command->cmd)
		{

		case NEW_USER:
		{
			lock_users(true);
			NewClientInfo *info = (NewClientInfo *)buffer;
			User *newUser = malloc(sizeof(User));
			newUser->pid = info->pid;
			newUser->beat = true;
			newUser->topics.head = NULL;
			strcpy(newUser->username, info->username);

			int wasRepeated = false;

			while (getUserNodeByUsername(newUser->username) != NULL)
			{
				wasRepeated = true;
				printf("Dupped name\n");

				// add number to username if dupped
				char *underline = &newUser->username[strlen(newUser->username)];
				int currentNumber = atoi(underline + 1);

				if (strncmp(underline, "_", 1) == 0)
				{ //if it has an underline
					sprintf(underline, "_%d", currentNumber + 1);
					printf("New number : %d\n", currentNumber + 1);
				}
				else
				{
					sprintf(underline, "_1");
				}
			}

			printf("[INFO]New Client; pid : %d , Username : %s\n", command->senderPid, newUser->username);

			int fd = open(info->pathToFifo, O_RDWR);
			if (fd == -1)
			{
				fprintf(stderr, "[ERROR]Error opening user fifo : Descarting User\n");
			}
			else
			{
				fprintf(stderr, "[INFO]Opened client fifo\n");
				newUser->fifo = fd;
				LinkedList_append(&cfg.users, newUser);

				if (wasRepeated)
				{
					sendToClient(newUser, USERNAME_REPEATED, newUser->username, USERNAME_L);
				}
				else
				{
					sendToClient(newUser, USERNAME_OK, NULL, 0);
				}
			}
			lock_users(false);
			break;
		}

		case NEW_MESSAGE:
		{
			lock_all(true);
			User *user = getUser(command->senderPid);
			Message *message = (Message *)buffer;
			int allowed = 1;
			printf("test 0\n");
			if (cfg.filter)
			{
				int badWordsCount = verifyBadWords(message);
				if (badWordsCount > cfg.maxbadWords)
				{
					allowed = 0;
					fprintf(stderr, "[INFO]Message Descarded, user : '%s', bad words: %d\n", user->username, badWordsCount);
					sendToClient(user, BAD_MESSAGE, NULL, 0);
					break;
				}
			}
			if (allowed)
			{
				Message *realMessage = malloc(sizeof(Message));
				memcpy(realMessage, message, sizeof(Message));
				realMessage->id = ++cfg.msgId;
				realMessage->duration = MESSAGE_DURATION;

				printf("test 1\n");

				LinkedList_append(&cfg.msgs, realMessage);
				Node *currTopic = cfg.topics.head;
				//check if topic exists
				int found = 0;
				while (currTopic != NULL)
				{
					if (currTopic->data != NULL)
					{
						char *topic = (char *)currTopic->data;
						if (strcmp(topic, realMessage->topic) == 0)
						{
							found = 1;
							break;
						}
					}
					currTopic = currTopic->next;
				}
				//add new topic if doesn't exist
				if (found == 0)
				{
					char *topic = malloc(sizeof(char) * TOPIC_L);
					strcpy(topic, realMessage->topic);
					LinkedList_append(&cfg.topics, topic);
				}

				// Send Notification to all subscribed clients
				MessageNotification notification;
				notification.id = realMessage->id;
				strncpy(notification.topic, realMessage->topic, TOPIC_L);
				Buffer buffer = prepareBuffer(MESSAGE_NOTIFICATION, &notification, sizeof(MessageNotification));
				printf("test 2\n");
				Node *currUserNode = cfg.users.head;
				while (currUserNode != NULL)
				{
					printf("test 3\n");
					User *currUser = (User *)currUserNode->data;
					printf("test 3 after\n");
					// If user is subscribed send notification
					if (getUserTopicNode(currUser, realMessage->topic) != NULL)
					{
						printf("test send buffer to . -> %s\n", currUser->username);
						sendBufferToClient(currUser, buffer);
					}
					currUserNode = currUserNode->next;
				}

				fprintf(stderr, "[INFO]New Message , user : '%s', id : '%d', title :'%s'\n", realMessage->username, realMessage->id, realMessage->title);
			}
			lock_all(false);
			break;
		}

		case USER_LEAVING:
		{
			lock_users(true);
			userLeft(getUserNode(command->senderPid));
			lock_users(false);
			break;
		}

		case SUBSCRIBE_TOPIC:
		{
			lock_users(true);
			lock_topics(true);
			User *user = getUser(command->senderPid);
			char *topic = buffer;
			Node *topicNode = getTopicNode(topic);

			if (topicNode != NULL)
			{ // if topic exists
				char *topicChar = (char *)topicNode->data;
				Node *userTopic = getUserTopicNode(user, topicChar);
				if (userTopic == NULL)
				{
					LinkedList_append(&user->topics, topicNode->data);
					fprintf(stderr, "[INFO]User \"%s\" subscribed to topic \"%s\"\n", user->username, topic);
					sendToClient(user, SUBSCRIBED_TO_TOPIC, NULL, 0);
				}
				else
				{
					sendToClient(user, ALREADY_SUBSCRIBED, NULL, 0);
				}
			}
			else
			{
				sendToClient(user, NON_EXISTENT_TOPIC, NULL, 0);
			}
			lock_users(false);
			lock_topics(false);
			break;
		}

		case UNSUBSCRIBE_TOPIC:
		{
			lock_users(true);
			lock_topics(true);
			User *user = getUser(command->senderPid);
			char *topic = buffer;

			if (deleteUserTopic(user, topic) == true)
			{
				fprintf(stderr, "User \"%s\" unsubscribed to topic \"%s\"\n", user->username, topic);
				sendToClient(user, UNSUBSCRIBE_TOPIC, NULL, 0);
			}
			else
				sendToClient(user, NON_EXISTENT_TOPIC, NULL, 0);

			lock_users(false);
			lock_topics(false);
			break;
		}

		case GET_TOPICS:
		{
			lock_topics(true);
			int topicsAmount = LinkedList_getSize(&cfg.topics);
			int totalBufferSize = sizeof(int) + topicsAmount * TOPIC_L;
			void *ptr = malloc(totalBufferSize);
			void *temp = ptr + sizeof(int);

			Node *curr = cfg.topics.head;
			for (int i = 0; i < topicsAmount && curr != NULL; i++)
			{
				void *pos = temp + i * TOPIC_L;
				memcpy(pos, curr->data, TOPIC_L);
				curr = curr->next;
			}

			lock_users(true);
			User *user = getUser(command->senderPid);
			lock_users(false);
			sendToClient(user, GET_TOPICS, ptr, totalBufferSize);
			lock_topics(false);
			break;
		}

		case HEARTBEAT_ISALIVE:
		{
			lock_users(true);
			User *user = getUser(command->senderPid);
			user->beat = true;
			lock_users(false);
			break;
		}

		default:
		{
			printf("Not Recognized Command\n");
			break;
		}
		}
	}
}

void *checkAllClientsState(void *data)
{
	while (1)
	{
		sleep(10);
		lock_users(true);
		Node *curr = cfg.users.head;
		while (curr != NULL)
		{
			User *user = (User *)curr->data;
			if (user->beat == false)
			{
				printf("User disconnected : %s\n", user->username);
				userLeft(curr);
			}
			user->beat = false;
			curr = curr->next;
		}
		lock_users(false);
	}
}

void *checkMessageTimeout(void *data)
{
	int const sleepTime = 1;
	while (true)
	{
		sleep(sleepTime);
		lock_msgs(true);
		Node *curr = cfg.msgs.head;
		while (curr != NULL)
		{
			Message *message = (Message *)curr->data;
			Node *next = curr->next;
			message->duration = message->duration - sleepTime;
			if (message->duration < 0)
			{
				//Delete message
				LinkedList_detachNode(&cfg.msgs, curr);
				free(curr->data);
				free(curr);
			}
			curr = next;
		}
		lock_msgs(false);
	}
}

int verifyBadWords(Message *message)
{
	write(cfg.sendVerif, message->title, strlen(message->title));
	write(cfg.sendVerif, "\n", 1);
	write(cfg.sendVerif, message->topic, strlen(message->topic));
	write(cfg.sendVerif, "\n", 1);
	write(cfg.sendVerif, message->body, strlen(message->body));
	write(cfg.sendVerif, "\n", 1);
	write(cfg.sendVerif, MSGEND, MSGEND_L);

	int const bufferSize = 4;
	char buffer[bufferSize];
	read(cfg.recieveVerif, buffer, bufferSize * sizeof(char));

	int nBadWords = atoi(buffer);
	return nBadWords;
}

void shutdown(int signal)
{
	printf("Exiting\n");
	unlink(LISTENER_PATH);
	Node *curr = cfg.users.head;
	while (curr != NULL)
	{
		User *currUser = (User *)curr->data;
		sendToClient(currUser, SERVER_SHUTDOWN, NULL, 0);
		curr = curr->next;
	}
	exit(0);
}

void userLeft(Node *node)
{
	LinkedList_detachNode(&cfg.users, node);
	// Limpar tudo acerca do utilizador
	User *user = (User *)node->data;
	// 1º limpar Nodes dos topicos a que pertence
	Node *currNode = user->topics.head;
	while (currNode != NULL)
	{
		Node *nextnode = currNode->next;
		free(currNode);
		currNode = nextnode;
	}
	// 2º limpar o user
	close(user->fifo);
	free(user);
	// 3º limpar o node do user
	free(node);
}

void sendToClient(User *user, int cmd, void *other, size_t size)
{
	Command command;
	command.cmd = cmd;
	command.senderPid = getpid();
	command.structSize = size;

	Buffer buffer = joinCommandStruct(&command, other, size);
	write(user->fifo, buffer.ptr, buffer.size);
	free(buffer.ptr);
}
Buffer prepareBuffer(int cmd, void *other, size_t size)
{
	Command command;
	command.cmd = cmd;
	command.senderPid = getpid();
	command.structSize = size;

	Buffer buffer = joinCommandStruct(&command, other, size);
	return buffer;
}
void sendBufferToClient(User *user, Buffer buffer)
{ //Used in loops
	write(user->fifo, buffer.ptr, buffer.size);
}

void printTopics(Node *head)
{
	Node *curr = head;
	printf("Topics : %d total\n#Topic -> users subscribed\n", LinkedList_getSize(&cfg.topics));

	while (curr != NULL)
	{ //TODO
		char *topic = (char *)curr->data;
		printf("\t%s ->", topic);

		Node *user_node = cfg.users.head;
		while (user_node != NULL)
		{
			User *user = (User *)user_node->data;
			if (getUserTopicNode(user, topic) != NULL)
				printf(" %s", user->username);
			user_node = user_node->next;
		}
		printf("\n");
		curr = curr->next;
	}
}
void printUsers(Node *head)
{
	Node *curr = head;
	printf("Users online : %d total\n\t#Name -> subscriptions\n", LinkedList_getSize(&cfg.users));

	while (curr != NULL)
	{
		User *user = (User *)curr->data;
		printf("\t%s ->", user->username);

		Node *currTopic = user->topics.head;
		while (currTopic != NULL)
		{
			printf(" %s", (char *)currTopic->data);
			currTopic = currTopic->next;
		}
		printf("\n");

		curr = curr->next;
	}
}
void printMsgs(Node *head)
{
	Node *curr = head;
	printf("Msgs on Memory : %d total\n", LinkedList_getSize(&cfg.msgs));

	while (curr != NULL)
	{
		Message *currMessage = (Message *)curr->data;
		printf("Id : %d,\n\tTitle : %s,\n\tUsername : %s,\n\tTopic : %s\n", currMessage->id, currMessage->title, currMessage->username, currMessage->topic);
		curr = curr->next;
	}
}

int messagesInTopic(char *topic)
{
	int count = 0;
	Node *currMsgNode = cfg.msgs.head;
	while (currMsgNode != NULL)
	{
		Message *message = (Message *)currMsgNode->data;

		if (strcmp(message->topic, topic) == 0)
		{
			++count;
		}

		currMsgNode = currMsgNode->next;
	}
	return count;
}

Node *getUserNode(pid_t pid)
{
	Node *curr = cfg.users.head;
	while (curr != NULL)
	{
		User *currUser = (User *)curr->data;
		if (currUser->pid == pid)
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}
User *getUser(pid_t pid)
{
	return (User *)getUserNode(pid)->data;
}
Node *getUserNodeByUsername(char *username)
{
	Node *curr = cfg.users.head;
	while (curr != NULL)
	{
		User *currUser = (User *)curr->data;
		if (strcmp(currUser->username, username) == 0)
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}
User *getUserByUsername(char *username)
{
	return (User *)getUserNodeByUsername(username)->data;
}
Node *getTopicNode(char *topic)
{
	Node *curr = cfg.topics.head;
	while (curr != NULL)
	{
		char *curr_topic = (char *)curr->data;
		if (strcmp(topic, curr_topic) == 0)
			return curr;
		curr = curr->next;
	}
	return NULL;
}
Node *getUserTopicNode(User *user, char *topic)
{
	Node *curr = user->topics.head;
	while (curr != NULL)
	{
		char *currTopic = (char *)curr->data;
		if (strcmp(currTopic, topic) == 0)
			return curr;
		curr = curr->next;
	}
	return NULL;
}
Node *getMessageNodeById(int id)
{
	Node *curr = cfg.msgs.head;
	while (curr != NULL)
	{
		Message *msg = (Message *)curr->data;
		if (msg->id == id)
			return curr;
		curr = curr->next;
	}
}
Message *getMessageById(int id)
{
	return (Message *)getMessageNodeById(id)->data;
}
int deleteUserTopic(User *user, char *topic)
{
	Node *userTopicNode = getUserTopicNode(user, topic);
	if (userTopicNode == NULL)
		return false;

	LinkedList_detachNode(&user->topics, userTopicNode);
	free(userTopicNode);
	return true;
}

void lock_m(pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
}
void unlock_m(pthread_mutex_t *mutex)
{
	pthread_mutex_unlock(mutex);
}
void lock_users(int bool)
{
	if (bool == true)
		lock_m(&cfg.mutex.usersLock);
	else
		unlock_m(&cfg.mutex.usersLock);
}
void lock_msgs(int bool)
{
	if (bool == true)
		lock_m(&cfg.mutex.msgsLock);
	else
		unlock_m(&cfg.mutex.msgsLock);
}
void lock_topics(int bool)
{
	if (bool == true)
		lock_m(&cfg.mutex.topicsLock);
	else
		unlock_m(&cfg.mutex.topicsLock);
}
void lock_all(int bool)
{
	lock_users(bool);
	lock_msgs(bool);
	lock_topics(bool);
}