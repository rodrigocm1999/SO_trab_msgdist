#include "comum.h"

int isServerRunning(){
    FILE* ptr = fopen(LISTENERPATH,"r");
    if(ptr == NULL)
        return 0;
    fclose(ptr);
    return 1;
}