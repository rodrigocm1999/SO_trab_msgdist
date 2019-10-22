#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>



int main(int argc,char* argv[]){


	char username[30];
	printf("Username: \n");
	scanf("%s",username);

	int choice;
	while(1){
		
		printf("\n1 - Write Message\n2 - List Topics\n3 - List Titles in Topic\n4 - Read Message in Topic\n5 - Subscribe to Topic\n6 - Unsubscribe from Topic\n0 - Exit\n");
		scanf("%d",&choice);

		switch (choice)
		{
		case 0: // Exit
			printf("Exiting\n");
			exit(0);
			break;
		case 1: // Write Message
			break;

		case 2: // List Topics
			break;

		case 3: // List Titles in Topic
			break;

		case 4: // Read Message in Topic
			break;

		case 5: // Subscribe to Topic
			break;

		case 6: // Unsubscribe to Topic
			break;

		default:
			printf("Invalid Option\n");
			break;
		}
		

	}




	/*initscr();
	cbreak();

	int nLines = 20,nCols = 100,y0 = 10,x0 = 10;
	WINDOW * win = newwin(nLines, nCols, y0, x0);
	
	wrefresh(win);


	while(1){pause();}*/

	return 0;
}
