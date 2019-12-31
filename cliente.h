#include "comum.h"
#include <locale.h>
#include "ncurses-menu.h"

struct windows
{
   WINDOW *topicsWindow;
   WINDOW *titlesFromTopicWindow;
   WINDOW *messageFromTopicWindow;
   WINDOW *borderTopicsWindow;
};



typedef struct{
   struct windows win;
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

