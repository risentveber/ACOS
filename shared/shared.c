#include <sys/shm.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


#include "mysem.h"

#define FILE_LENGTH  0x1000
#define BUFF_SIZE 128 // FILE_LENGTH > buffsize + 100
#define DATA_SIZE 50000
#define EX_COUNT 100
#define END_OF_TUBE -1
const char * file =  "shared_file";

struct tube
{
	int begin;
	int middle;
	int end; 

	int res_data;
	int emp_data;
	int wrt_data;

	int stop_write;
	int stop_read;

	int arr[BUFF_SIZE];
};

void tube_init(struct tube * t)
{
	t->stop_read = 0;
	t->stop_write = 0;
	t->begin = 0;
	t->middle = 0;
	t->end = 0;
	t->res_data = 0;
	t->wrt_data = 0;
	t->emp_data = 1;
}

struct span
{
	int left;
	int right;
};


void file_creating()
{
	int fd;
	//creating a tmp-file
	fd = shm_open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0){
		perror("cann't create shared_file");
		exit(1);
	}

	if (ftruncate(fd, FILE_LENGTH)){
		perror("cann't change the length of shared_file");
		exit(1);
	};
	close(fd);
}

int file_preparing()
{
	int fd;
	fd = shm_open(file, O_RDWR,  0);
	if (fd < 0){
		perror("cann't open shared_file");
		exit(1);
	}

	return fd;
}

void * mmap_preparing()
{
	int fd = file_preparing();
	void* memory;

	memory = mmap (0, FILE_LENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (memory == MAP_FAILED){
		perror("mmap failed");
		exit(1);
	}

	close (fd);
	return memory;
}

key_t sem_name1 = 233546248;
key_t sem_name2 = 231251328;
key_t sem_name3 = 135245128;

int stop_read, stop_write, test;


int distance(int l, int r) // [l, r) - здесь и далее интервалы такого вида
{
	if (l <= r)
		return r - l;
	else 
		return BUFF_SIZE - l + r;
}

void how_sum(struct tube *t, struct span *s, int count)
{
	int yes = 0;
	while (!yes){
		bin_sem_wait(stop_read);

		bin_sem_wait(test);
		if (t->wrt_data){
			yes = 1;
			int c = distance(t->begin, t->middle);
			if (c == 0 && t->wrt_data)
			c = BUFF_SIZE;

			if (c < count)
				count = c;
			s->left = t->begin;
			s->right = (t->begin + count)%BUFF_SIZE;
		} else 
			t->stop_read = 1;
		
		bin_sem_post(test);
	}
	bin_sem_post(stop_read);
	return;
}

void how_write(struct tube *t, struct span *s, int count)
{
	int yes = 0;
	while (!yes){
		bin_sem_wait(stop_write);
		bin_sem_wait(test);
		if (t->emp_data){
			yes = 1;
			int c = distance(t->end, t->begin);
			if (c == 0 && t->emp_data)
				c = BUFF_SIZE;

			if (c < count)
				count = c;

			t->res_data = 1;
			s->left = t->end;
			t->end = (t->end + count)%BUFF_SIZE;//
			s->right = t->end;
		
		} else 
			t->stop_write = 1;	

		bin_sem_post(test);
	}
	bin_sem_post(stop_write);
}
	


void free_sum(struct tube *t, struct span *s)
{
	bin_sem_wait(test);
	
	t->begin = s->right;
	t->emp_data = 1;

	if(distance(t->begin, t->middle)){
		t->wrt_data = 1;
	} else
		t->wrt_data = 0;
	
	if(t->stop_write){
		t->stop_write = 0;
		bin_sem_post(stop_write);
	}
	
	bin_sem_post(test);
	return;
}

void set_write(struct tube *t, struct span *s)
{
	bin_sem_wait(test);

	t->middle = s->right;
	
	t->wrt_data = 1;
	t->res_data = 0;

	if(distance(t->end, t->begin)){
		t->emp_data = 1;
	}else
		t->emp_data = 0;

	if(t->stop_read){
		t->stop_read = 0;
		bin_sem_post(stop_read);
	}

	bin_sem_post(test);
	return;
}

int sum_from_tube(struct tube * t, int * sum, int num)
{
	int i;
	struct span s;
	how_sum(t, &s, num);

	int d = distance(s.left, s.right);//интервал обязательно не нулевой
	if (d == 0)
		d = BUFF_SIZE;
	
	for(i = 0; i < d; i++)
		*sum += t->arr[(i + s.left) %BUFF_SIZE];
	
	free_sum(t, &s);

	return d;
}

int write_to_tube(struct tube * t, int * val, int num)
{
	int i;
	struct span s;
	how_write(t, &s, num);

	int d =  distance(s.left, s.right);
	if (d == 0)
		d = BUFF_SIZE;

	for(i = 0; i < d; i++) 
		t->arr[(s.left + i)%BUFF_SIZE] = val[i];
	
	set_write(t, &s);

	return d;
}

int min(int a, int b)
{
	if (a > b)
		return b;
	else 
		return a;
}

int main()
{
	//создание разделяемой памяти
	file_creating();
	void* shared_memory = mmap_preparing();	
	tube_init((struct tube *)shared_memory);

	printf("buffsize: %d\n", BUFF_SIZE);
	printf("datasize: %d\n", DATA_SIZE);

	//создание семафоров и ветвление
	stop_read = bin_sem_alloc(sem_name1);
	stop_write = bin_sem_alloc(sem_name2);
	test = bin_sem_alloc(sem_name3);
	if (stop_write < 0 || stop_read < 0 || test < 0 ||
		bin_sem_init(stop_write) < 0 ||  bin_sem_init(stop_read) < 0 || bin_sem_init(test) < 0){
		perror("semaphore allocation failed");
		exit(1);
	}

	pid_t child = fork();

	long long int i;
	int result = 0;
	int writed = 0;
	int summed = 0;
	int c;
	/*
	if (child > 0){// parent
		int a[DATA_SIZE];		
		srand(time(NULL));
		for(c = 0; c < DATA_SIZE; c++)		
			a[c] =  rand();

		struct timeval t1, t2;
		long long int diff;
		int num = 0;
		printf("%5s%10s%15s%15s\n", "num", "size","average_time", "speed[Mb/s]");
		for (i = 1; i <= DATA_SIZE; i = i*2){
			num++;
			gettimeofday(&t1, NULL);
			for(c = 0; c < EX_COUNT; c++){
				writed = 0;
				while (writed < i)
					writed += write_to_tube((struct tube *)shared_memory, a + writed, i - writed);
			}
			gettimeofday(&t2, NULL);
			diff = (t2.tv_sec - t1.tv_sec)*1000000 + t2.tv_usec - t1.tv_usec;           
			printf("%5d%10lld%15lf%15lf \n", num, i*sizeof(int),
				(double)(diff)/EX_COUNT, EX_COUNT*i*sizeof(int)/(double)(diff)/1048576*1000000);
		}
		wait(NULL);
	} else if (child == 0){ //child
		
		for (i = 1; i <= DATA_SIZE; i = i*2){
			for(c = 0; c < EX_COUNT; c++){
				summed = 0;
				while (summed < i)
					summed += sum_from_tube((struct tube *)shared_memory, &result, i - summed);
			}
		}

	} else { // something bad has occured
		perror("cann't fork");
	}*/

	//#################### проверка работы сумматора##############################
	if (child > 0){// parent
		int a[DATA_SIZE];		
		
		int h;
		result = 0;
		for(h = 0; h < DATA_SIZE; h++)
			result += a[h];
		printf("parent: %d\n", result);
		writed = 0;
		while (writed < DATA_SIZE)
			writed += write_to_tube((struct tube *)shared_memory, a + writed, min(37,DATA_SIZE - writed));
		wait(NULL);
	} else if (child == 0){ //child
		
		result = 0;
		summed = 0;
		while (summed < DATA_SIZE)
			summed += sum_from_tube((struct tube *)shared_memory, &result, min(45,DATA_SIZE - summed));
		printf("child : %d\n", result);

	} else { // something bad has occured
		perror("cann't fork");
	}
	//#################### проверка работы сумматора##############################
	munmap(shared_memory, FILE_LENGTH);
	bin_sem_free(stop_read);
	bin_sem_free(stop_write);
	bin_sem_free(test);
	shm_unlink(file);
	return 0;
}
