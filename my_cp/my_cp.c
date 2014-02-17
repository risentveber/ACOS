#include <unistd.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

void print_usage()
{	
	char * message = "usage : program file1 file2\n";
	
	int length = 0;
	while(message[length++]);

	write(2, message, length);
	_exit(1);
}

int main(int argc, char** argv)
{	
	if (argc != 3)
		print_usage();
	char * filename1 = argv[1];
	char * filename2 = argv[2];
	int fd1, fd2;
	
	fd1 = open(filename1,O_RDONLY);
	if (fd1 < 0){
		perror("problems file1");
		_exit(1);
	}

	fd2 = open(filename2, O_WRONLY | O_CREAT | O_TRUNC );
	if (fd2 < 0){
		perror("problems file2");
		close(fd1);
		_exit(1);
	}

	char buffer[BUFF_SIZE];
	int readed;
	int writed;

	do {
		readed = read(fd1, buffer, BUFF_SIZE);
		if (readed > 0){
			writed = write(fd2, buffer, readed);
			if (writed != readed){
				perror("problems with writing into file2");
				close(fd1);
				close(fd2);				
				_exit(1);
			}			
		}else if (readed < 0){
			perror("problems with reading from file1");
			close(fd1);
			close(fd2);
			_exit(1);
		}
		
	} while (readed); // nothing has been readed

	close(fd1);
	close(fd2);

	
	return 0;

}
