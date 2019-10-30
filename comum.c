#include "comum.h"

int isServerRunning(){
    int f = open(LISTENERPATH,O_RDWR);
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
    dest = dest + sizeof(Command);
    memcpy(dest,other,otherSize);
    return buffer;
}



/*int isRunning(){
	int fd = open(LISTENERPATH,O_RDWR);
    if(fd == -1){
        return 0;
    }
    close(fd);
	 return 1;
	/*int deleted = unlink(LISTENERPATH);
	if(deleted != 0) {// non success

		int fd = open(LISTENERPATH,O_WRONLY);
		if(fd == -1){ 
			return 0;
		}
		close(fd);
		return 1;
	}
   return deleted == 0 ? 0 : 1;

	if( access( LISTENERPATH, F_OK ) != -1 ) {
		
	}
}*/