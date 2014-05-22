//серверная часть
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tmp.h"


#define PORT 7890	// the port users will be connecting to

int main(void) {
	struct message MMM;
	int sock, new_sockfd;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_in host_addr, client_addr;	// my address information
	socklen_t sin_size;
	int recv_length=1, yes=1;
	char buffer[1024];

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		fatal("in socket");
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		fatal("setting socket option SO_REUSEADDR");
	
	host_addr.sin_family = AF_INET;		 // host byte order
	host_addr.sin_port = htons(PORT);	 // short, network byte order
	host_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(host_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(sock, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
		fatal("binding to socket");
	if (listen(sock, 5) == -1)
		fatal("listening to socket");
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	fd_set rfds;
	int resselect;
	std::vector<int> masssocket;
	masssocket.push_back(sock);
	FD_ZERO (&rfds);
	int maxfd;

	while (1)
	{
		maxfd = 0;
		for (int i = 0; i < masssocket.size(); ++i){
			if (masssocket[i] > maxfd)
				maxfd=masssocket[i];
			FD_SET(masssocket[i], &rfds);
		}
		resselect=select(maxfd+1, &rfds, NULL, NULL, NULL);
		if (resselect== -1)
			perror("ERSELECT");
		else{
			if (FD_ISSET(sock, &rfds)){
				printf("connection established\n");
				int fd = accept(sock, NULL, NULL); 
				masssocket.push_back(fd);

			}
			
			for (int i = 1; i < masssocket.size(); ++i)
				if (FD_ISSET(masssocket[i], &rfds))
				{
					if (!read_message(masssocket[i], &MMM)){
						printf("connection is broken\n");
						masssocket.erase(masssocket.begin() + i);
						continue;
					}

					for (int j = 1; j < i; ++j)
						write_message(masssocket[j], &MMM);
					for (int j = i + 1; j < masssocket.size(); ++j)
						write_message(masssocket[j], &MMM);
				}
		}
	}

	return 0;
}


