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
#define BUFF_SIZE 100 // FILE_LENGTH > buffsize + 100
#define DATA_SIZE 1000
#define END_OF_TUBE -1
const char * file =  "shared_file";


struct tube
{
	int bad;
	// [][][][]begin[][][][]middle[][][][][][]end[][][][]
	int begin;
	int middle;
	int end; 

	int arr[BUFF_SIZE];
};

void tube_init(struct tube * t)
{
	t->begin = 0;
	t->middle = 0;
	t->end = 0;
	t->bad = 0;
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

key_t sem_name1 = 233546242;
key_t sem_name2 = 231251325;
key_t sem_name3 = 135245125;

int full, empty, test;//семафоры



int distance(int l, int r) // [l, r) - здесь и далее интервалы такого вида
{
	if (l <= r)
		return r - l;
	else 
		return BUFF_SIZE - l + r;
}

void how_sum(struct tube *t, struct span *s, int count)
//записывает в span не нулевой интервал для суммирования не больше count
{
	bin_sem_wait(empty);//подождать когда что-то появится

	bin_sem_wait(test);

	
	int c = distance(t->begin, t->middle);
	if (c == 0)//потому как буфер не пуст
		c = BUFF_SIZE;
	if (c < count)
		count = c;
	s->left = t->begin;
	s->right = (t->begin + count)%BUFF_SIZE;
	if(distance( s->right, t->middle))//если что-то осталось не считанное, поднять семафор наличия данных
		bin_sem_post(empty);

	bin_sem_post(test);
}

void free_sum(struct tube *t, struct span *s)
//освобождает span - интервал для записи
{
	bin_sem_wait(test);

	t->begin = s->right;
	bin_sem_post(full);// мы что-то считали поэтому буффер не полон

	bin_sem_post(test);

}

void how_write(struct tube *t, struct span *s, int count)
//записывает в span не нулевой интервал зарезервированный для записи не больше count
{
	bin_sem_wait(full);//подождать когда появится место

	bin_sem_wait(test);

	int c = distance(t->end, t->begin);
	if (c == 0)
		c = BUFF_SIZE;
	if (c < count)
		count = c;
	s->left = t->end;
	t->end = (t->end + count)%BUFF_SIZE;
	s->right = t->end;

	if(distance( t->end, t->begin))//если что-то осталось пустое место, поднять семафор наличия места
		bin_sem_post(full);

	bin_sem_post(test);
}

void set_write(struct tube *t, struct span *s)
//устанавливает span - интервал для записанным
{
	bin_sem_wait(test);

	t->middle = s->right;
	bin_sem_post(empty);// мы что-то записали поэтому буффер не пуст

	bin_sem_post(test);

}

int sum_from_tube(struct tube * t, int * sum, int num)
{
	//if(t->bad)
	//	return END_OF_TUBE;

	int i;
	struct span s;
	how_sum(t, &s, num);
	int d = distance(s.left, s.right);
	if (d == 0)
		d = BUFF_SIZE;
	for(i = 0; i < d; i++){
		*sum += t->arr[(i + s.left) %BUFF_SIZE];
	}

	free_sum(t, &s);

	return d;

}

int write_to_tube(struct tube * t, int * val, int num)
{
	//if(t->bad)
	//	return END_OF_TUBE;

	struct span s;

	how_write(t, &s, num);
	
	int i;
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
	full = bin_sem_alloc(sem_name1);
	empty = bin_sem_alloc(sem_name2);
	test = bin_sem_alloc(sem_name3);
	if (full < 0 || empty < 0 || test < 0 ||
		bin_sem_init(full) < 0 ||  bin_sem_init(empty) < 0 || bin_sem_init(test) < 0){
		perror("semaphore allocation failed");
		exit(1);
	}
	
	bin_sem_wait(empty);//изначально буфер пуст

	pid_t child = fork();

	int i;
	int result = 0;
	int writed = 0;
	int summed = 0;

	if (child > 0){// parent
		int a[DATA_SIZE];		
		
		for(i = 0; i < DATA_SIZE; i++)
				result += a[i];

		while (writed < DATA_SIZE){
			//for(i = 0; i < 50; i++)
			//	result += a[i];
			writed += write_to_tube((struct tube *)shared_memory, a + writed, min(50, DATA_SIZE - writed));
			//printf("parent : writed %d : result %d\n", writed, result );
		}
		printf("zzparent:result: %d\n", result);
		
	} else if (child == 0){ //child
		while (summed < DATA_SIZE){
			summed += sum_from_tube((struct tube *)shared_memory, &result, min(10, DATA_SIZE - summed));
			//if(summed % 50 == 0)
				//printf("child : summed %d : result %d\n", summed, result );
		}
		printf("zzchild:result: %d\n", result);

	} else { // something bad has occured
		perror("cann't fork");
	}

	munmap(shared_memory, FILE_LENGTH);
	bin_sem_free(full);
	bin_sem_free(empty);
	bin_sem_free(test);
	shm_unlink(file);
	return 0;
}
