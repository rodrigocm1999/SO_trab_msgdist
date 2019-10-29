#include "comum.h"

int isServerRunning(){
    FILE* ptr = fopen(LISTENERPATH,"r");
    if(ptr == NULL)
        return 0;
    fclose(ptr);
    return 1;
}

void joinCommandWithOther(Command* command,void* other, size_t otherSize){
    char buffer[sizeof(Command) + otherSize + 1];

    //buffer = (*command);


}