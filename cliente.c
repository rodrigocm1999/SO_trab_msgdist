#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>



int main(int argc,char* argv[]){

	initscr();
	cbreak();

	int nLines = 20,nCols = 100,y0 = 10,x0 = 10;
	WINDOW * win = newwin(nLines, nCols, y0, x0);
	
	wrefresh(win);


	while(1){pause();}

	return 0;
}
