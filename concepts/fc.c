#include "utils.h"
void navigate(void)
{
	int scry, scrx;
	struct window *currWindow = defaultWindow();
	char command;
	while(1) {
		getmaxyx(stdscr, scry, scrx);
		int maxNameSize = currWindow->maxNameSize;
		int lineFiles = filesPerLine(scrx, maxNameSize);
		if(lineFiles > 0 && scrx > 30 && scry > 10) {
			struct dirent **allfiles = currWindow->allFiles;
			struct line L = getFirstLine(allfiles, lineFiles);
			int totalLines = goesIntoNum(totalFiles(allfiles), lineFiles, 0);	
			screenRender(L, totalLines, lineFiles, scry, scrx, 
				     maxNameSize, allfiles, currWindow->currFile);
			//printw("%i", totalFiles(L.file));
			command = getch();
			if(command == 'f') {
				leaveDirectory(currWindow->directory, L, currWindow->allFiles);
				free(currWindow);
				return;
			}
			else if(command == 'l' && *(currWindow->currFile + 1) != NULL)
				++currWindow->currFile;
			else if(command == 'h' && currWindow->currFile != allfiles)
				--currWindow->currFile;
			else if(command == 'd') {
				erase();
				printw("%s", (*currWindow->currFile)->d_name);
				getch();
			}
			else if(command == 'e') {
			        struct window *tmp = changeDirectory((*currWindow->currFile)->d_name);
				if(tmp != NULL)
					currWindow = tmp;
			} else if(command = 'j') {
				incLine(L, currWindow->allFiles, lineFiles);
				renderLine(L, currWindow->currFile, maxNameSize);
			}
			//cleanUpLine(L);
		} else {
			//stops the screen from going crazy in resizing
		       	getch();
			erase();
		}
	}
}

void init(void)
{
	initscr();
	noecho();
	cbreak();
	curs_set(0);
	start_color();			/* Start color 			*/
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
}

int main(void)
{
	
	init();
	navigate();
	changeDirectory("..");
	navigate();
	endwin();
}
