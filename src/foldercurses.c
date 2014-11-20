#include <dirent.h> 
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>

struct dirent **files;
struct dirent **lastfile;
struct dirent **currfile;
DIR *d;
int maxNameSize;
int currline;
int numOfFiles;

enum{allView, Upsome, Downsome, UpDownsome};
int offScreens;
int numOfLines(int scrx)
{
	return (numOfFiles * maxNameSize) / scrx;
}

int filePos(struct dirent **infile) {
	int i = 1;
	struct dirent **ptr;
	for(ptr = files; ptr != infile; ++ptr) {
		++i;
	}
	return i;
}

int getFileLine(int Pos, int scrx)
{
	(Pos * maxNameSize) / scrx;
}

int shouldRender(struct dirent **infile, int totalLines, int scry, int scrx)
{
	if(totalLines < scry) {
		return 1;
	}
	int fileplace = getFileLine(filePos(infile), scrx);
	int currfileoff = getFileLine(filePos(currfile), scrx);
	int topbound = currfileoff + scry/2;
	int bottombound = currfileoff - scry/2;
	if(fileplace < bottombound)
	{
		if(offScreens == Downsome)
			offScreens = UpDownsome;
		else if(offScreens == allView)
			offScreens = Upsome;
	}
	if(fileplace > topbound)
	{
		if(offScreens == Upsome)
			offScreens = UpDownsome;
		else if(offScreens == allView)
			offScreens = Downsome;
	}
		       
	if(fileplace > bottombound && fileplace < topbound) {
		return 1;
	}

	return 0;

}
int curslen(void) {
	int i = 0;
	struct dirent **ptr;
	for(ptr = files; *ptr != NULL; ++ptr) {
		++i;
	}
	return i;
}

struct dirent **fileslast(void)
{
	struct dirent **ptr;
	for(ptr = files; *ptr != NULL; ++ptr);
	return ptr;

}

void setup(void)
{
	files = malloc(sizeof(struct dirent *));
	*files = NULL;
	return;
}

void cleanup(void)
{
	closedir(d);
	free(files);
	return;
}

void loadDirectory(void)
{
	maxNameSize = 0;
	numOfFiles = 0;
	setup();
	struct dirent *dir;
	if (d) {
		while((dir = readdir(d)) != NULL) {
			files = realloc(files,  (curslen() + 2) * (sizeof(struct dirent *)));
			lastfile = fileslast();
			*(lastfile) = dir;
			*(++lastfile) = NULL;
			if(strlen(dir->d_name) > maxNameSize)
				maxNameSize = strlen(dir->d_name);
			++numOfFiles;
		}	
	}
	maxNameSize += 5;
	currfile = files;
	return;
}

void changeDirectory(char *newd)
{
	if((chdir(newd)) == -1)
	{
		erase();
		printw("Unable to switch");
		noecho();
		curs_set(0);
		cbreak();
		getch();
		erase();
		return;
	}
	cleanup();
	d = opendir(".");
	loadDirectory();
	return;
}

void bottomPanel(int scry, int scrx)
{
	struct passwd *pw;
	uid_t uid;

	move(scry -1, 0);
	int i;
	attron(A_STANDOUT);
	for(i = 0; i < (scrx + 1); ++i)
		printw("-");
	attroff(A_STANDOUT);

	uid = geteuid ();
	pw = getpwuid (uid);
	move(scry, 0);
	printw(pw->pw_name);
	if(strcmp(pw->pw_name, "root") == 0)
		printw("#");
	else
		printw("$");
	printw(" ");

	move(scry -1, scrx/2);
	if(offScreens == allView) {
		printw("A");
	} else if(offScreens == UpDownsome) {
		printw("X");
	} else if(offScreens == Upsome) {
		printw("U");
	} else if(offScreens == Downsome) {
		printw("D");
	}
}

void render(int scry, int scrx)
{
	int words;
	words = 0;
	int newlinenum = 0;
	struct stat buf;
	struct dirent **srcRender = files;
	int totalLines = numOfLines(scrx);
	offScreens = allView;
	while(*srcRender) {
		if(shouldRender(srcRender, totalLines, scry, scrx)) {
			int currNameSize = strlen((*srcRender)->d_name);
			if(words + maxNameSize > scrx) {
				printw("\n", maxNameSize);
				words = 0;
				++newlinenum;
			}
			stat((*srcRender)->d_name, &buf);
			if(!S_ISREG(buf.st_mode))
				attron(COLOR_PAIR(1));
			if(srcRender == currfile) {
				words += maxNameSize;
				attron(A_BOLD | A_UNDERLINE);
				printw("%s", (*srcRender)->d_name);
				attroff(A_BOLD | A_UNDERLINE);
			} else {
				words += maxNameSize;
				printw("%s", (*srcRender)->d_name);
			}
			int i = 0;	
			while(i + currNameSize < maxNameSize && words < scrx) {
				printw(" ");
				++i;
			}
		}
		++srcRender;
		attroff(COLOR_PAIR(1));
	}
	bottomPanel(scry, scrx);
}

void insertShell(int scry, int scrx)
{

	struct passwd *pw;
	uid_t uid;
	uid = geteuid ();
	pw = getpwuid (uid);
	echo();
	curs_set(1);
	nocbreak();
	move(scry, strlen(pw->pw_name) + 2);
	
	char* shcomm = malloc(2);
	*shcomm = getch();
	char newchar;
	if(*shcomm != '\n') {
		*(shcomm + 1) = '\0';
		char *shlastchar = shcomm;
		while((newchar = getch()) != '\n') {
			shcomm = realloc(shcomm, strlen(shcomm) + 2);
			*(++shlastchar) = newchar;
			*(shlastchar + 1) = '\0';
		}
		if(strncmp(shcomm, "cd ", 3) == 0) {
			changeDirectory(shcomm + 3);
		} else {
			system(shcomm);
		}
	}
        free(shcomm);
	noecho();
	curs_set(0);
	cbreak();

}



void navigate(void)
{
	int command;
	int scry, scrx;
	while(1) {
		getmaxyx(stdscr, scry, scrx);
		--scry;
		--scrx;
		render(scry, scrx);
		command = getch();
		
		if(command == 'l' && *(currfile + 1) != NULL)
			++currfile;
		if(command == 'h' && currfile != files)
			--currfile;
		if(command == 'a')
			currfile = lastfile - 1;
		if(command == 'o')
			changeDirectory("..");
		if(command == 'e')
			changeDirectory((*currfile)->d_name);
		if(command == 'r')
			changeDirectory(".");
		if(command == 'f')
			return;
		if(command == 'i')
			insertShell(scry, scrx);
		erase();

	}
}
int main(void)
{
	initscr();

	start_color();			/* Start color 			*/
	init_pair(1, COLOR_CYAN, COLOR_BLACK);

	cbreak();
	noecho();
	curs_set(0);
	d = opendir(".");
	loadDirectory();
	navigate();
	endwin();                  /* End curses mode    */
	/*
	struct dirent **newfiles = files;
	while(*newfiles != NULL) {
		printf("%s\n", (*newfiles)->d_name);
		++newfiles;
	}
	changeDirectory("opencv-1.0.0");
	newfiles = files;
	while(*newfiles != NULL) {
		printf("%s\n", (*newfiles)->d_name);
		++newfiles;
	}
	*/
	cleanup();
	return 0;
}
 
