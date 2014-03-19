#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>


void print_usage(const char *program_name)
{
	printf("%s\n", program_name);
	printf("\t%s [program_name]", program_name);
}

int main(int argc, char const *argv[])
{
	unsigned int p;
	if (argc <= 2 || 0 >= (p = atoi(argv[1])) || p > 100 ) {
		print_usage(argv[0]);
		exit(1);
	}
	pid_t child;

	child = fork();

	int flag = 1;

	if(child){
		while (waitpid(child, NULL, WNOHANG) == 0) { 
			if (flag){
				kill(child, SIGCONT);
				usleep(p*10000U);
			} else {
				kill(child, SIGSTOP);
				usleep((100 - p)*10000U);
			}
			flag = !flag;
		}
	} else {
		char const ** a = (char const **)malloc((argc - 1)*sizeof(void *));
		if (a){
			int i;
			for(i = 0; i < argc - 2; ++i)
				a[i] = argv[i + 2];
			a[argc -2] = NULL;
			execvp(argv[2], (char * const *)a);
		}else{
			perror("can't allocate memory");
			exit(1);
		}
	}

	return 0;
}


