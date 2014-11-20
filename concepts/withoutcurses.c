#include <dirent.h> 
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>

struct dirent **files;
struct dirent **lastfile;
struct dirent **currfile;
DIR *d;

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
	setup();
	struct dirent *dir;
	if (d) {
		while((dir = readdir(d)) != NULL) {
			files = realloc(files,  (curslen() + 2) * (sizeof(struct dirent *)));
			lastfile = fileslast();
			*(lastfile) = dir;
			*(++lastfile) = NULL;
		}	
	}
	currfile = files;
	return;
}

void changeDirectory(char *newd)
{
	if((chdir(newd)) == -1)
	{
		printf("Unable to switch");
		return;
	}
	cleanup();
	d = opendir(".");
	loadDirectory();
	return;
}

void navigate(void)
{
	
	while(1) {
		char command = getchar();
		while(command == '\n')
			command = getchar();
		
		if(command == 'l' && *(currfile + 1) != NULL)
			++currfile;
		if(command == 'h' && currfile != files)
			--currfile;
		if(command == 'e')
			changeDirectory((*currfile)->d_name);
		printf("%s", (*currfile)->d_name);
	}
}
int main(void)
{

	d = opendir(".");
	loadDirectory();
	navigate();
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
 
