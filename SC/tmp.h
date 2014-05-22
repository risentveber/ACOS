#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME_SIZE 20
#define TEXT_SIZE 100



// A function to display an error message and then exit
void fatal(const char *message) {
   char error_message[100];

   strcpy(error_message, "[!!] Fatal Error ");
   strncat(error_message, message, 83);
   perror(error_message);
   exit(-1);
}

struct message
{
	char name[NAME_SIZE];
	char text[TEXT_SIZE];	
};

int read_message(int fd, struct message *m)
{
	int readed = 0;
	int size = sizeof(struct message);
	int d;
	while (readed < size){
		d = 0;
		d = read(fd, (char *)m + readed, size - readed);
		if (d <= 0){
			if (d < 0)
				perror("read_message: read");
			return 0;
		}else 
			readed += d;

	}
	return 1;
}

int write_message(int fd, struct message *m)
{
	int writed = 0;
	int size = sizeof(struct message);
	int d;
	while (writed < size){
		d = 0;
		d = write(fd, (char *)m + writed, size - writed);
		if (d <= 0){
			if (d < 0)
				perror("write_message: write");
			return 0;
		}else 
			writed += d;

	}
	return 1;
}

void scanf_message(struct message *m, char *name)
{
	strcpy(m->name, name);
	fgets(m->text, TEXT_SIZE, stdin);
}

#define C_Reset      0
#define C_Bold       1
#define C_Under      2
#define C_Invers     3
#define C_Normal     4
#define C_Black      4
#define C_Red        5
#define C_Green      6
#define C_Brown      7
#define C_Blue       8
#define C_Magenta    9
#define C_Cyan       10
#define C_Light      11
#define N_COLORS     C_Light+1

static int tcolor [N_COLORS] = {0,  1,  4,  7, 30, 31, 32, 33, 34, 35, 36, 37 };
static char buff[10];

void print_message(int color, struct message* m)
{
	write(1, buff, sprintf (buff,"\033[%dm",tcolor[color]));
	write(1, m->name, strlen(m->name));
	write(1, ":", 1);
	write(1, buff, sprintf (buff,"\033[%dm",tcolor[0]));	
	write(1, m->text, strlen(m->text));
}
