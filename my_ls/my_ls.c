//file: my_ls.c
//file_creator: risent
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>
#include <time.h>

#include "queue.h"

void print_usage(const char * name)
{
	printf("usage %s is:\n", name);
	printf("\t%s [flags] [directory_name] ...\n", name);
	printf("flags:\n");
	printf("\t-l --list print full information\n");
	printf("\t-a --all  print all files\n");
	printf("\t-h --help print help message\n");
	printf("\t-r --recursively print recursively\n");
	printf("\n");
}

/*
struct stat {
               mode_t    st_mode;    / protection
               uid_t     st_uid;     / user ID of owner
               gid_t     st_gid;     / group ID of owner /
               off_t     st_size;    / total size, in bytes /
               time_t    st_atime;   / time of last access /
               time_t    st_mtime;   / time of last modification /
               time_t    st_ctime;   / time of last status change /
           };
*/

void print_full_file_info(const char* file_name, const struct stat * f_stat)
{
	struct passwd *tmp_pwd;
	struct tm tmp_time;
	time_t time;
	mode_t mode = f_stat->st_mode;
	putchar('\t');
	if(S_ISDIR(mode))
		putchar('d');
	else if(S_ISLNK(mode))
		putchar('l');
	else
		putchar('-');

	int i;
	for (i = 8; i >= 0; --i)
	{
		if (mode & 1 << i){
			if((i+1)% 3 == 0)
				putchar('r');
			if((i+1)% 3 == 1)
				putchar('x');
			if((i+1)% 3 == 2)
				putchar('w');
		}else 
			putchar('-');
	}
	
	
	tmp_pwd = getpwuid(f_stat->st_uid);
	if (tmp_pwd){
		time = f_stat->st_mtime;
		char * time_str = ctime(&time);
		int i=0;
		{//delete next-line symbol in time_str
			while(time_str[++i]);
			--i;
			time_str[i] = '\0'; 
		}
		printf(" %2d", (int)(f_stat->st_nlink));     //print number of hard links
		printf(" %s", tmp_pwd->pw_name);             //print username
		printf(" %7ld", (long int)(f_stat->st_size));//print size of file
		printf(" %s", time_str);                     //print time of last modification
		
	}
	printf(" %s\n", file_name);

}


void print_dir(const char* program_name, const char *base_dir_name, int flag_l, int flag_a, int flag_r)
{
	struct dirent * tmp_name;
	struct stat tmp_stat;
	char full_file_name[256];
	char dir_name[256];

	struct queue my_queue;
	init_queue(&my_queue);
	int good;
	push_queue(&my_queue, base_dir_name);

	while (!queue_is_empty(&my_queue)) {
		pop_queue(&my_queue, dir_name);

		DIR * dir = opendir(dir_name);

		if (dir == NULL){
			fprintf(stderr, "%s: problems with directory ", program_name);
			perror(dir_name);
			continue;
		}

		tmp_name = readdir(dir);
		
		printf("%s contains:\n", dir_name);
		while (tmp_name){
			good = 0;
			if (tmp_name->d_name[0] != '.' || flag_a) {//print all files if -a has been passed
				if (flag_l){// print full information
					strcpy(full_file_name, dir_name);
					strcat(full_file_name, "/");
					strcat(full_file_name, tmp_name->d_name);
					if(lstat(full_file_name, &tmp_stat) == 0) {
						good = 1;
						print_full_file_info(tmp_name->d_name, &tmp_stat);
					} else 
						perror(tmp_name->d_name);
				} else
					printf("\t%s\n", tmp_name->d_name);
			}
			if (flag_r) {
				if (!flag_l){// if lstat hasn't been called
					strcpy(full_file_name, dir_name);
					strcat(full_file_name, "/");
					strcat(full_file_name, tmp_name->d_name);
					if(lstat(full_file_name, &tmp_stat) == 0)
						good = 1;
					else 
						perror(tmp_name->d_name);
				}
				if (good && strcmp(tmp_name->d_name, "..") && strcmp(tmp_name->d_name, ".") && S_ISDIR(tmp_stat.st_mode))
					push_queue(&my_queue, full_file_name);
			}
			tmp_name = readdir(dir);
		}

		closedir(dir);
	}

}



int main(int argc, char* argv[])
{

	int next_option;
	const char* short_options = "hlar";

	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"list", 0, NULL, 'l'},
		{"all", 0, NULL, 'a'},
		{"recursively", 0, NULL, 'r'},
		{ NULL, 0, NULL, 0}
	};

	int flag_l = 0;
	int flag_a = 0;
	int flag_r = 0;
	const char *program_name = argv[0];

	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		switch (next_option) {
			case 'h' :
				print_usage(program_name);
				exit(0);
			case 'l' :
				flag_l = 1;
				break;
			case 'a' :
				flag_a = 1;
				break;
			case 'r' :
				flag_r = 1;
				break;
			case '?' :
				print_usage(program_name);
				exit(1);
			case -1 :
				break;
			default :

				abort();
		}
	} while (next_option != -1); // end of options processing

	if (argc == optind){ //working directory is implicit parameter
		print_dir(program_name, ".", flag_l, flag_a, flag_r);
	}

	for(int i = optind; i < argc; i++){
		print_dir(program_name, argv[i], flag_l, flag_a, flag_r);
	}


	return 0;
}
