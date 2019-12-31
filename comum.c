#include "comum.h"

int isServerRunning(){
    int f = open(LISTENER_PATH,O_RDWR);
    if(f == -1){
        return 0;
    }
    close(f);
    return 1;
}


Buffer joinCommandStruct(Command* command,void* other, size_t otherSize){
    int bufferSize = sizeof(Command) + otherSize ;
    Buffer buffer;
    buffer.ptr = malloc(bufferSize);
    buffer.size = bufferSize;
    void * dest = buffer.ptr;
    
    memcpy(dest,command,sizeof(Command));
    if(otherSize != 0 && other != NULL){
        dest = dest + sizeof(Command);
        memcpy(dest,other,otherSize);
    }
    return buffer;
}