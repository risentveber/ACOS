#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>
#include <dirent.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

void print_usage(const char * program_name)
{	
	printf("usage of %s\n", program_name);
	printf("\t%s [file_source] [file_destination]\n", program_name);
	printf("\t%s [file_source] ... [dir_destination]\n", program_name);	
}

const char * get_filename(const char * path)
{
	int l = strlen(path);
	int i;
	int flag = 0;
	for(i = l - 1; i >= 0; --i){
		if(path[i] == '/'){
			if (flag)
				break;
			
		} else 
			flag = 1;
	}

	if (i < 0)
		if (strcmp(path, ".") && strcmp(path, "..") && strcmp(path, "~"))
			return path;
		else
			return NULL;
	else 
		return &(path[i]);
}

void copy_file_to_file(const char * full_source_filename, const char * full_dest_filename)// dest always is file
{
	int fd1, fd2;
	struct stat tmp_stat;

	if(lstat(full_source_filename, &tmp_stat) != 0){
		perror(full_source_filename);
		return;
	}

	

	fd1 = open(full_source_filename, O_RDONLY);
	if (fd1 < 0){
		perror(full_source_filename);
		return;
	}

	fd2 = open(full_dest_filename, O_WRONLY | O_CREAT | O_TRUNC, tmp_stat.st_mode);
	if (fd2 < 0){
		perror(full_dest_filename);
		close(fd1);
		return;
	}

	char buffer[BUFF_SIZE];
	int readed;
	int writed;

	do {
		readed = read(fd1, buffer, BUFF_SIZE);
		if (readed > 0){
			writed = write(fd2, buffer, readed);
			if (writed != readed){
				perror(full_dest_filename);
				close(fd1);
				close(fd2);				
				return;
			}			
		}else if (readed < 0){
			perror(full_source_filename);
			close(fd1);
			close(fd2);
			return;
		}
		
	} while (readed); // nothing has been readed

	close(fd1);
	close(fd2);
	return;
}

void copy_dir_to_dir(const char * full_source_filename, const char * full_dest_filename)// dest always is dir
{
	struct stat tmp_stat;
	char new_full_filename[256];
	char tmp_full_filename[256];
	const char *filename = get_filename(full_source_filename); // if NULL return
	if (filename == NULL){
		fprintf(stderr, "this realisation doesn't support  only ~ . .. directories %s\n", full_source_filename);
		return;
	}
	
	if(lstat(full_source_filename, &tmp_stat) == 0) {
			strcpy(new_full_filename, full_dest_filename);
			strcat(new_full_filename, "/");
			strcat(new_full_filename, filename);
			
			if(S_ISDIR(tmp_stat.st_mode)){
				mkdir(new_full_filename, tmp_stat.st_mode);//if error has occured we will see it in next lines
				DIR * dir = opendir(full_source_filename);

				if (dir == NULL){
					fprintf(stderr, "cann't creat directory %s", new_full_filename);
					perror("");
					return;
				}

				struct dirent * tmp_name;
				tmp_name = readdir(dir);
				while (tmp_name){
					if (tmp_name->d_name[0] != '.') {
							strcpy(tmp_full_filename, full_source_filename);
							strcat(tmp_full_filename, "/");
							strcat(tmp_full_filename, tmp_name->d_name);

						copy_dir_to_dir(tmp_full_filename, new_full_filename);
					}
					tmp_name = readdir(dir);
				}

				closedir(dir); 

			} else {
				copy_file_to_file(full_source_filename, new_full_filename);
			}
	} else {
		perror(full_source_filename);
		return;
	}

}

void option_processing(int argc , char** argv)
{
	int next_option;
	const char *short_options = "h";

	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{ NULL, 0, NULL, 0}
	};

	const char *program_name = argv[0];

	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		switch (next_option) {
			case 'h' :
				print_usage(program_name);
				exit(0);
			case '?' :
				print_usage(program_name);
				exit(1);
			case -1 :
				break;
			default :
				abort();
		}
	} while (next_option != -1); // end of options processing



	if (argc - optind <= 1){
		print_usage(program_name);
		exit(1);
	}
}

int main(int argc, char** argv)
{	
	option_processing(argc, argv);

	const char *full_dest_filename = argv[argc - 1];
	const char *full_source_filename;
	int dest_is_dir;
	int source_is_dir;
	struct stat tmp_stat;

	dest_is_dir = 0;
	source_is_dir = 0;
	if(lstat(full_dest_filename, &tmp_stat) == 0)
			dest_is_dir = S_ISDIR(tmp_stat.st_mode);
	
	if (argc - optind == 2){ // [file_sorce] [file_destination] || [dir_destination]

		full_source_filename = argv[argc - 2];

		if (lstat(full_dest_filename, &tmp_stat) == 0)
			source_is_dir = S_ISDIR(tmp_stat.st_mode);

		if (source_is_dir && !dest_is_dir)
			fprintf(stderr, "cann't copy directory %s to file %s\n", full_source_filename, full_dest_filename);
		else if (dest_is_dir)
			copy_dir_to_dir(full_source_filename, full_dest_filename);
		else
			copy_file_to_file(full_source_filename, full_dest_filename);

	} else { // [file_source] ... [dir_destination]
		if (!dest_is_dir) {
			fprintf(stderr, "cann't copy files to unexisting directory (or permission denied) %s\n", full_dest_filename);
			exit(1);
		}

		int i;
		for(i = optind; i < argc - 1; ++i){
			copy_dir_to_dir(argv[i], full_dest_filename);
		}

	}

	
	return 0;

}
