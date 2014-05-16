#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

const int BUFF_SIZE = 1000000;
const int max_size = 1000000000;
const int ex_count = 100;
char * buff;

int write_to_pipe(int fd, int c)
{
	int writed = 0;
	int tmp;
	while(writed != c){
		tmp = c - writed;
		if (tmp / BUFF_SIZE)
			tmp = BUFF_SIZE;
		else
			tmp = tmp % BUFF_SIZE;
		tmp = write(fd, buff, tmp);
		if (tmp < 0){
			perror("cann't write to pipe");
			return 1;
		}
		writed += tmp;

	}
	return 0;
}

int read_from_pipe(int fd, int c)
{	
	int readed = 0;
	int tmp;
	while(readed != c){
		tmp = c - readed;
		if (tmp / BUFF_SIZE)
			tmp = BUFF_SIZE;
		else
			tmp = tmp % BUFF_SIZE;
		tmp = read(fd, buff, tmp);
		if (tmp < 0){
			perror("cann't read from pipe");
			return 1;
		}
		readed += tmp;

	}
	return 0;
}

int main(int argc, char const *argv[])
{
	buff = (char *)malloc(BUFF_SIZE);
	if (buff == NULL){
		perror("cann't allocate memory");
		exit(1);
	}
	memset(buff, 1, BUFF_SIZE);

	int p[2];
	if (pipe(p)){
		perror("cann't create pipe");
		exit(1);
	}



	pid_t pid = fork();
	long long int i;
	int c;

	if ( pid > 0) { //parent
		close(p[1]);//close end to write 
		int fd_read = p[0];
		struct timeval t1, t2;
		long long int diff;
		int num = 0;
		printf("%5s%10s%15s%10s\n", "num", "size","average_time", "speed");
		for (i = 1; i <= max_size; i = i*2){
			num++;
			gettimeofday(&t1, NULL);
			for(c = 0; c < ex_count; c++){
				read_from_pipe(fd_read, i);
			}
			gettimeofday(&t2, NULL);
			diff = (t2.tv_sec - t1.tv_sec)*1000000 + t2.tv_usec - t1.tv_usec;
			printf("%5d%10lld%15lf%10lf \n", num, i, (double)(diff)/ex_count, ((double)(diff))/ex_count/i);
		}
		close(fd_read);
	} else if (pid == 0) { //child
		close(p[0]);//close end to read 
		int fd_write = p[1];

		for (i = 1; i <= max_size; i = i*2){
			for(c = 0; c < ex_count; c++){
				write_to_pipe(fd_write, i);
			}
		}

		close(fd_write);
	} else {//something is wrong
		perror("cann't fork");
		exit(1);
	}



	free(buff);

	return 0;
}