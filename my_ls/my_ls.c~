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

void print_usage(const char * name)
{
	printf("usage %s is:\n", name);
	printf("\t%s [flags] [directory_name] ...\n", name);
	printf("flags:\n");
	printf("\t-l --list print full information\n");
	printf("\t-a --all  print all files\n");
	printf("\t-h --help print help message\n");
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

	//owner permissions
	if (mode & S_IRUSR)
		putchar('r');
	else
		putchar('-');

	if (mode & S_IWUSR)
		putchar('w');
	else
		putchar('-');

	if (mode & S_IXUSR)
		putchar('x');
	else
		putchar('-');
		
	//group permissions
	if (mode & S_IRGRP)
		putchar('r');
	else
		putchar('-');

	if (mode & S_IWGRP)
		putchar('w');
	else
		putchar('-');

	if (mode & S_IXGRP)
		putchar('x');
	else
		putchar('-');
		
	//others permissions
	if (mode & S_IROTH)
		putchar('r');
	else
		putchar('-');

	if (mode & S_IWOTH)
		putchar('w');
	else
		putchar('-');

	if (mode & S_IXOTH)
		putchar('x');
	else
		putchar('-');
	
	
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


void print_dir(const char* program_name, const char *dir_name, int flag_l, int flag_a)
{
	DIR * dir = opendir(dir_name);

	if (dir == NULL){
		fprintf(stderr, "%s: problems with directory ", program_name);
		perror(dir_name);
		return;
	}

	struct dirent * tmp_name;
	struct stat tmp_stat;
	char full_file_name[256];


	tmp_name = readdir(dir);

	printf("%s contains:\n", dir_name);
	while (tmp_name){
		if (tmp_name->d_name[0] != '.' || flag_a) {//print all files if -a has been passed
			if (flag_l){// print full information
				strcpy(full_file_name, dir_name);
				strcat(full_file_name, "/");
				strcat(full_file_name, tmp_name->d_name);
				if(stat(full_file_name, &tmp_stat) == 0)
					print_full_file_info(tmp_name->d_name, &tmp_stat);
				else 
					perror(tmp_name->d_name);
			} else
				printf("\t%s\n", tmp_name->d_name);
		}
		tmp_name = readdir(dir);
	}

	closedir(dir);

}



int main(int argc, char* argv[])
{

	int next_option;
	const char* short_options = "hla";

	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"list", 0, NULL, 'l'},
		{"all", 0, NULL, 'a'},
		{ NULL, 0, NULL, 0}
	};

	int flag_l = 0;
	int flag_a = 0;
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
		print_dir(program_name, ".", flag_l, flag_a);
	}

	for(int i = optind; i < argc; i++){
		print_dir(program_name, argv[i], flag_l, flag_a);
	}


	return 0;
}
