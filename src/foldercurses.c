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
		getch();
		erase();
		return;
	}
	cleanup();
	d = opendir(".");
	loadDirectory();
	return;
}

void bottomPanel(int srcy, int srcx)
{
	struct passwd *pw;
	uid_t uid;

	move(srcy -1, 0);
	int i;
	attron(A_STANDOUT);
	for(i = 0; i < (srcx + 1); ++i)
		printw("-");
	attroff(A_STANDOUT);

	uid = geteuid ();
	pw = getpwuid (uid);
	move(srcy, 0);
	printw(pw->pw_name);
	if(strcmp(pw->pw_name, "root") == 0)
		printw("#");
	else
		printw("$");
	printw(" ");
}

void render(int srcy, int srcx)
{
	int words;
	words = 0;
	int newlinenum = 0;
	struct stat buf;
	struct dirent **srcRender = files;	
	while(*srcRender) {
		int currNameSize = strlen((*srcRender)->d_name);
		if(words + currNameSize > srcx) {
			printw("\n", maxNameSize);
			words = 0;
			++(newlinenum);
		}	struct stat buf;
		
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
		while(i + currNameSize < maxNameSize && words < srcx) {
			printw(" ");
			++(words);
			++i;
		}
		++srcRender;
		attroff(COLOR_PAIR(1));
	}
	bottomPanel(srcy, srcx);
}

void insertShell(int srcy, int srcx)
{

	struct passwd *pw;
	uid_t uid;
	uid = geteuid ();
	pw = getpwuid (uid);
	echo();
	curs_set(1);
	nocbreak();
	move(srcy, strlen(pw->pw_name) + 2);
	
	char* shcomm = malloc(2);
	*shcomm = getch();
	if(*shcomm != '\n') {
		*(shcomm + 1) = '\0';
		char *shlastchar = shcomm;
		while(*shlastchar != '\n') {
			shcomm = realloc(shcomm, strlen(shcomm) + 2);
			*(++shlastchar) = getch();
			*(shlastchar + 1) = '\0';
		}
		system(shcomm);
	}
        free(shcomm);
	noecho();
	curs_set(0);
	cbreak();

}



void navigate(void)
{
	int command;
	int srcy, srcx;
	while(1) {
		getmaxyx(stdscr, srcy, srcx);
		--srcy;
		--srcx;
		render(srcy, srcx);
		command = getch();
		
		if(command == 'l' && *(currfile + 1) != NULL)
			++currfile;
		if(command == 'h' && currfile != files)
			--currfile;
		if(command == 'e')
			changeDirectory((*currfile)->d_name);
		if(command == 'f')
			return;
		if(command == 'i')
			insertShell(srcy, srcx);
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
 
