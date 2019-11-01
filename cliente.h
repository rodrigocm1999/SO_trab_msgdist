#include "comum.h"
#include <ncurses.h>


typedef struct{
   int server;
   int fifo;
   char username[USERNAME_L];
   char fifoPath[64];
}ClientConfig;



void* fifoListener(void* data);
void shutdown();
int sendToServer(int cmd,void* other,size_t size);