#include "comum.h"
#include <locale.h>
#include "ncurses-menu.h"


typedef struct{
   int server;
   int fifo;
   int server_running;
   char username[USERNAME_L];
   char fifoPath[64];
}ClientConfig;


void *checkServerRunning(void *data);
void* fifoListener(void* data);
void shutdown();
void signalHandler(int signal);
void* heartbeat(void* data);
int sendToServer(int cmd,void* other,size_t size);