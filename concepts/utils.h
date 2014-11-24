#ifndef FUTILS_H
#define FUTILS_H

#include <dirent.h> 
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>

struct line
{
	int *number;
	struct dirent **file;
};

struct window
{
	char *currDirect;
	int maxNameSize;
	struct dirent **allFiles;
	struct dirent **currFile;
	DIR *directory;
};
int goesIntoNum(int large, int small, int offset)
{
	int i;
	for(i = 0; i + small + offset <= large; i += small);
	return i/small;
}

int filesPerLine(int scrx, int maxNameSize)
{
	return goesIntoNum(scrx, maxNameSize, 1);
}

int totalFiles(struct dirent **allFiles)
{
	struct dirent **fileHolder;
	int i = 0;
	for (fileHolder = allFiles; *fileHolder != NULL; ++fileHolder)
		++i;
	//erase();
	//printw("%i", i);
	//getch();
	return i;
}

void incLine(struct line L, struct dirent **allFiles, int FilesPerLine)
{

	struct dirent **fileHolder;
	for(fileHolder = allFiles; *fileHolder != *L.file; ++fileHolder);
	
	fileHolder += FilesPerLine;

	struct dirent **fileHolder2;
	int i = 0;
	for(fileHolder2 = L.file; *fileHolder2 != NULL; ++fileHolder2) {
		*fileHolder2 = *(fileHolder + i);
		++i;
	}
	++(*L.number);
}

struct line getFirstLine(struct dirent **allfiles, int filesInLine)
{
	struct line L;
	L.file = malloc((filesInLine + 1)* (sizeof(struct dirent **)));
	struct dirent **fileHolder = allfiles;
	struct dirent **fileHolder2 = L.file;
	int i;
	for(i = 0; i != filesInLine; ++i) {
		*fileHolder2++ = *fileHolder++;
	}
	*fileHolder2 = NULL;
	L.number = malloc(sizeof(int));
	*(L.number) = 0;
	return L;
}

void equalizeSpacing(char *name, int maxNameSize)
{
	int i = strlen(name);
	while(i != maxNameSize) {
		printw(" ");
		++i;
	}
}

int isFolder(char *fileName)
{
	struct stat buf;
	stat(fileName, &buf);
	if(!S_ISREG(buf.st_mode))
		return 1;
	return 0;
}

int getMaxNameSize(struct dirent **allFiles)
{
	struct dirent **fileHolder;
	int largestNameSize = 0;
	for(fileHolder = allFiles; *fileHolder != NULL; ++fileHolder) {
		int templength = strlen((*fileHolder)->d_name);
		if(templength > largestNameSize)
			largestNameSize = templength;
	}
	return largestNameSize;
}

void renderLine(struct line L, struct dirent **currFile, int maxNameSize)
{
	struct dirent **fileHolder;
	char *fileName;
	for(fileHolder = L.file; *fileHolder != NULL; ++fileHolder) {
		fileName = (*fileHolder)->d_name;
		if(*fileHolder == *currFile) {
			attron(A_BOLD | A_UNDERLINE);
		}
		if(isFolder(fileName)) {
			attron(COLOR_PAIR(1));
		}
		printw("%s", fileName);
		attroff(A_BOLD | A_UNDERLINE);
		attroff(COLOR_PAIR(1));
		equalizeSpacing(fileName, maxNameSize);
		
		
	}
	printw("\n");
		
}

struct dirent **fileslast(struct dirent **allfiles)
{
	struct dirent **ptr;
	for(ptr = allfiles; *ptr != NULL; ++ptr);
	return ptr;

}

int curslen(struct dirent **files) {
	int i = 0;
	struct dirent **ptr;
	for(ptr = files; *ptr != NULL; ++ptr) {
		++i;
	}
	return i;
}

struct dirent **loadDirectory(DIR *d)
{
	struct dirent **files;
	struct dirent *dir;
	struct dirent **lastfile;
	if (d) {
		if((dir = readdir(d)) != NULL) {
			files = malloc(sizeof(struct dirent *) * 2);
			*files = dir;
			*(files + 1) = NULL;
		}
		while((dir = readdir(d)) != NULL) {
			files = realloc(files,  (curslen(files) + 2) * (sizeof(struct dirent *)));
			lastfile = fileslast(files);
			*(lastfile) = dir;
			*(++lastfile) = NULL;
		}	
	}

	return files;
}

void cleanUpLine(struct line L)
{
	free(L.file);
	free(L.number);
	L.file = NULL;
}

void leaveDirectory(DIR *d, struct line L, struct dirent **allfiles)
{
	free(L.file);
	free(L.number);
	L.file = NULL;

	free(allfiles);
	closedir(d);
}

int lineContainsFile(struct line L, struct dirent **currFile)
{
	struct dirent **fileHolder;
	int solution = 0;
	for(fileHolder = L.file; *fileHolder != NULL; ++fileHolder) {
		if(*fileHolder == *currFile)
			solution = 1;
	}

	return solution;
}

struct line copyLine(struct line firstline, int lineFiles)
{
	struct line lineHolder;
	lineHolder.number = malloc(sizeof(int));
	lineHolder.file = malloc(sizeof(struct dirent *) * (lineFiles + 1));
	*lineHolder.number = *firstline.number;
	struct dirent **fileHolder;
	struct dirent **fileHolder2 = lineHolder.file;
	for(fileHolder = firstline.file; *fileHolder != NULL; ++fileHolder) {
		*fileHolder2++ = *fileHolder;
	}
	*fileHolder2 = NULL;
	return lineHolder;
}

int currLineNum(struct line firstLine, struct dirent **currFile, struct dirent **allFiles, int lineFiles)
{
	struct line lineHolder = copyLine(firstLine, lineFiles);
	int should_not_exit = 1;
	int solution;
	while(should_not_exit) {
		if(lineContainsFile(lineHolder, currFile)) {
			solution = *lineHolder.number;
			should_not_exit = 0;
		} else {
			incLine(lineHolder, allFiles, lineFiles);			
/*erase();
			printw("%s\n%s", (*lineHolder.file)->d_name, (*currFile)->d_name);
			getch();*/
		}
	}
	cleanUpLine(lineHolder);
	return solution;
}

void fillLine(char *text, int scrx)
{
	int i;
	int textlen = strlen(text);
	for(i = 0; i != scrx;) {
		if(i == (scrx/2) - textlen/2) {
			printw("%s", text);
			i += textlen;
		}
		printw("-");
		++i;
	}
}

void renderTopPanel(int scrx)
{
	attron(A_STANDOUT);
	fillLine("Folder Curses v. 1.0.0", scrx);
	attroff(A_STANDOUT);

}

void renderBottomPanel(int scry, int scrx)
{
	attron(A_STANDOUT);
	move(scry - 2, 0);
	fillLine("Folder Curses v. 1.0.0", scrx);
	attroff(A_STANDOUT);
}

void renderLineNumbers(struct line L, struct dirent **currFile,
		       struct dirent **allFiles, int maxNameSize,
		       int lineFiles, int totalLines, int lowerbound, int upperbound)
{
	while(*L.number <= totalLines) {
	       	if(*L.number >= lowerbound && *L.number <= upperbound)
			renderLine(L, currFile, maxNameSize);
	       	incLine(L, allFiles, lineFiles);
	}
}

void screenRender(struct line L, int totalLines, int lineFiles,
		  int scry, int scrx, int maxNameSize, struct dirent **allFiles,
		  struct dirent **currFile)
{
	move(0,0);
        clear();
	renderTopPanel(scrx);
	int currscry = scry - 3;
	int lineOfCurrent = currLineNum(L, currFile, allFiles, lineFiles);
	int whatLayer = goesIntoNum(lineOfCurrent, currscry, 0);
	int lowerbound = (whatLayer * currscry);
	int upperbound = (lowerbound + currscry);
	renderLineNumbers(L, currFile, allFiles, maxNameSize, 
			  lineFiles, totalLines, lowerbound, upperbound);
	renderBottomPanel(scry, scrx);

	refresh();
}
char *currentDirectory(void)
{
	char cwd[1024];
	char *returnedDirectory;
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
	        returnedDirectory = malloc(strlen(cwd) + 1);
		strcpy(returnedDirectory, cwd);
		
	} else {
		erase();
		printw("Error, could not find current directory");
		exit(1);
	}
	return returnedDirectory;
}
struct window *defaultWindow(void)
{
	struct window *W = malloc(sizeof(struct window));
	W->directory = opendir(".");
	W->allFiles = loadDirectory(W->directory);
	W->currFile = W->allFiles;
	W->maxNameSize = getMaxNameSize(W->allFiles) + 2;
	W->currDirect = currentDirectory();
	
	return W;

}
//char *getDirectory(struct window *currWindow)
void freeWindowContents(struct window *W)
{
	free(W->allFiles);
	free(W->currDirect);
	closedir(W->directory);
}
struct window *changeDirectory( char *newDirectory)
{

	if((chdir(newDirectory)) == -1)
	{
		erase();
		printw("Unable to switch");
		noecho();
		curs_set(0);
		cbreak();
		getch();
		erase();
		return NULL;
	}
	
	//freeWindowContents(currWindow);
        return defaultWindow();
}
#endif
