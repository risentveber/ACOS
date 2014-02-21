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

void print_usage(const char * name)
{
	printf("usage %s is:\n", name);
	printf("\t%s [-l] [directory_name] ...\n", name);
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
	mode_t mode = f_stat->st_mode;
	
	if(S_ISDIR(mode))
		putchar('d');
	else if(S_ISLNK(mode))
		putchar('d');
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

	printf(" %s\n", file_name);

}


void print_dir(const char* program_name, const char *dir_name, int flag_l)
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
		//if (tmp_name->d_name[0] != '.') {
			putchar('\t');
			if (flag_l){// print full information
				strcpy(full_file_name, dir_name);
				strcat(full_file_name, tmp_name->d_name);
				stat(full_file_name, &tmp_stat);
				print_full_file_info(tmp_name->d_name, &tmp_stat);
			} else
				printf("%s\n", tmp_name->d_name);
		//}
		tmp_name = readdir(dir);
	}

	closedir(dir);

}



int main(int argc, char* argv[])
{

	int next_option;
	const char* short_options = "hl";

	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"list", 0, NULL, 'l'},
		{ NULL, 0, NULL, 0}
	};

	int flag_l = 0;
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
		print_dir(program_name, ".", flag_l);
	}

	for(int i = optind; i < argc; i++){
		print_dir(program_name, argv[i], flag_l);
	}


	return 0;
}
