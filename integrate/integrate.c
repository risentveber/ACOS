#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BASE_PARTION 100

double f(double x)
{
	return x*x*x;
}

double n_part(int k)
{
	char name[20];
	double result;
	sprintf(name, "/tmp/int%d", k);
	FILE * file = fopen(name,"r");
	if (file != NULL)
	{
		fscanf(file, "%lf", &result);
		fclose(file);
		return result;
	}else{
		perror("cann't open to read");
		return 0;
	}
}

void calculate(double a, double b, int k)
{
	char name[20];
	double result = 0;
	sprintf(name, "/tmp/int%d", k);
	double d_x = (b - a)/BASE_PARTION;
	for (int i = 0; i < BASE_PARTION; ++i)
	{
		result += d_x * f(a + ((b - a)*i)/BASE_PARTION);
	}

	FILE * file = fopen(name,"w");
	if (file != NULL)
	{
		fprintf(file, "%lf", result);
		fclose(file);
	} else
		perror("cann't open to write");
}

int main(int argc, char const *argv[])
{
	double result = 0;
	int pid = 10;
	int started = 0;
	int n = 10;//atoi(argv[1]);
	if (n < 0){
		printf("usage");
	}
	double tmp_a, tmp_b;
	double a = 0;
	double b = 10;
	int * child_pids = (int *)malloc(sizeof(int) * n);
 	if (child_pids == NULL)
 		exit(1);

	while(started != n){
		tmp_a = a + ((b - a)*started)/n;
		tmp_b = a + ((b - a)*(started + 1))/n;
		pid = fork();
		if (pid > 0){// parent
			child_pids[started] = pid;
			started++;
		} else if (pid == 0){//child
			calculate(tmp_a, tmp_b, started);
			return 0;
		} else {//something is bad
			free(child_pids);
			exit(1);
		}
	}

	for (int i = 0; i < n; ++i)
		waitpid(child_pids[i], NULL, 0);
	for (int i = 0; i < n; ++i)
		result += n_part(i);
	printf("the integral is %lf\n", result);

	free(child_pids);
	return 0;
}