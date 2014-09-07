#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include "tmp.h"

int main(int argc, char **argv)
{
	if (argc < 2)
		fatal("too few arguments");
	char *client_name = argv[1];
	struct message MMM;

	if ( strlen(client_name) + 1 > NAME_SIZE)
		fatal("very long client name");

	int sock;
	u_int16_t port;
	long tmp_port;
	struct sockaddr_in sin;
	tmp_port = 7890;

	port = htons(tmp_port);
	sin.sin_family = AF_INET; 
	sin.sin_port = port;

	{
		struct hostent* host = gethostbyname("127.0.0.1");
		if (host == NULL)
			fatal("hostname is bad");
		memcpy(&sin.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		fatal("cann't create socket");

	if (connect(sock,(struct sockaddr*)&sin, sizeof(struct sockaddr_in))) 
		fatal("cann't connect with server");
	printf("connection established\n");


	fd_set base, rfds;
	int resselect;
	FD_ZERO (&base);
	FD_SET(sock, &base);
	FD_SET(0, &base);
	int is_quit = 0;		

	while (!is_quit)
	{
		rfds = base;
		resselect = select(sock + 1, &rfds, NULL, NULL, NULL);
		if (resselect == -1)
			perror("select error");
		else
		{
			if (FD_ISSET(0, &rfds))
			{
				scanf_message(&MMM, client_name);
				write_message(sock, &MMM);
				if (!strcmp("exit\n", MMM.text)){
					is_quit = 1;
					continue;
				}
			}
			if (FD_ISSET(sock, &rfds))
			{
				read_message(sock, &MMM);
				print_message(6, &MMM);
			}
		}
	}
	close(sock);
	return 0;
}
