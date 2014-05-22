#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "tmp.h"
#include <vector>
#include <set>

using namespace std;

#define PORT 7890
#define SERVER_IP "93.175.2.156"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
vector<int>	client_fd;
vector<unsigned> client_IP;
const char *client_name;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void print_ip(unsigned ip) 
{
    struct in_addr ip_addr;
    ip_addr.s_addr = ip;
    printf("%s\n", inet_ntoa(ip_addr));
}

int connect_to_server(unsigned server_name)
{
	struct sockaddr_in server;
	int sock;

	server.sin_family = AF_INET; 
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = server_name;

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		fatal("Error socket");
	//printf("client: socket: SUCCESS1\n");

	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
		fatal("Errro connect");	
	//printf("client: connect: SUCCESS2\n");
	printf("connection to ");
	print_ip(server_name);
	return sock;
}

int connect_to_server(const char * server_name)
{
	struct hostent* host;
	if ((host = gethostbyname(server_name)) == NULL)
		fatal("Error hostname");
	unsigned IP;
	memcpy(&IP, host->h_addr_list[0], host->h_length);
	return connect_to_server(IP);			
}

int create_server()
{
	struct sockaddr_in server;
	int listen_sock;

	memset(&server, 0, sizeof(server));
	server.sin_family = PF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;

	if ((listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		fatal("creation socket error");
	//printf("server: socket: SUCCESS\n");

	int yes=1;//set the adress can be reused
	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
		fatal("setsockopt failed");
	//printf("server: setsockopt: SUCCESS\n");

	if (bind(listen_sock, (struct sockaddr*)&server, sizeof(server)) < 0)
		fatal("bind failed");
	//printf("server: bind: SUCCESS\n");

	if (listen(listen_sock, 10) < 0)
		fatal("listen failed");
	//printf("server: listen: SUCCESS\n");
	
	printf("server has created\n");
	return listen_sock;
}

unsigned ip_from_char(const char *str)
{
	struct hostent* host;
	if ((host = gethostbyname(str)) == NULL)
		fatal("Error hostname");
	unsigned IP;
	memcpy(&IP, host->h_addr_list[0], host->h_length);
	return IP;
}

int get_service_message(struct message *m)
{
	if (!(m->is_service))
		return 0;
	int i;
	unsigned new_client;
	unsigned new_fd;
	for(i = 0; i < MAX_IP && *((unsigned *)(m->text) +i); i++){
		new_client = *((unsigned *)(m->text) + i);
		printf("adding new client ");
		print_ip(new_client);
		new_fd = connect_to_server(new_client);
		write(new_fd, "0", 1);
		client_IP.push_back(new_client);
		client_fd.push_back(new_fd);
	}


	return 1;
}

int send_clients(int fd)
{
	struct message m;
	

	int c_count = client_IP.size();
	int sended = 1; // listen_socket is vector[0]
	int tmp_send;
	printf("%d\n", c_count);
	while(sended < c_count)
	{
		tmp_send = (c_count - sended)%MAX_IP;
		tmp_send = (tmp_send)?tmp_send:MAX_IP;
		bzero((char *)&m, sizeof(m));
		m.is_service = 1;
		strcpy(m.name, client_name);
		int i;
		for(i = 0; i < tmp_send; i++){
			printf("send ");
			print_ip(client_IP[sended + i]);
			*((unsigned *)(m.text) + i) = client_IP[sended + i];
		}
		write_message(fd, &m);
		sended += tmp_send;
	}

}

void new_connection()
{
	struct sockaddr_in new_client;
	socklen_t len = sizeof(new_client);
	int new_fd;
	unsigned new_IP;

	new_fd = accept(client_fd[0], (struct sockaddr *)&new_client, &len); 
	//отправление всех своих соседей
	new_IP = new_client.sin_addr.s_addr;
	char a;
	read(new_fd, &a, 1);
	if (a == '1'){
	printf("sending clients to " );
	print_ip(new_IP);
	send_clients(new_fd);
}
	//получение ип адреса
	client_fd.push_back(new_fd);
	client_IP.push_back(new_IP);	
}
void listen_from_socket()
{

	struct message MMM;
	fd_set rfds;
	int resselect;
	FD_ZERO (&rfds);
	int maxfd = 0;

	//возводим все сокеты
	for (int i = 0; i < client_fd.size(); ++i)
	{
		if (client_fd[i] > maxfd)
			maxfd=client_fd[i];
		FD_SET(client_fd[i], &rfds);
	}
	//возводим чтение из стд
	FD_SET(0, &rfds);

	resselect = select(maxfd+1, &rfds, NULL, NULL, NULL);
	if (resselect== -1)
		perror("select error");
	else
	{
		if (FD_ISSET(client_fd[0], &rfds))
		{
			printf("new connection\n");
			new_connection();
		}
		if (FD_ISSET(0, &rfds))
		{
			//прочитали с клавиатуры
			scanf_message(&MMM, client_name);
			//разослали всем
			for (int i=1; i<client_fd.size(); ++i)
				write_message(client_fd[i], &MMM);
			if (!strcmp("exit\n", MMM.text))
				exit (0);
		}
		for (int i = 1; i < client_fd.size(); ++i)
			if (FD_ISSET(client_fd[i], &rfds))
			{
				if (!read_message(client_fd[i], &MMM))
				{
					printf("connection is broken\n");
					close(client_fd[i]);
					client_fd.erase(client_fd.begin() + i);
					client_IP.erase(client_IP.begin() + i);
					continue;
				}
				if(!get_service_message(&MMM))				
					print_message(6, &MMM);
			}
	}	
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main(int argc, char** argv)
{
	struct sockaddr_in cur_client;
	socklen_t len = sizeof(sockaddr_in);
	int cur_client_fd;
	unsigned cur_client_IP;
	int listen_sock;

	if (argc>3 || argc == 1)
		fatal("too many or few arguments");

	client_name = argv[1];

	//for listen socket that will stored into 0 position
	client_IP.push_back(unsigned());
	client_fd.push_back(int());

	if (argc == 3)
	{
		cur_client_fd = connect_to_server(argv[2]);
		client_fd.push_back(cur_client_fd);
		unsigned tmp = ip_from_char(argv[2]);
		client_IP.push_back(tmp);
		write(cur_client_fd, "1", 1);
		printf("Сonnection to CHAT NET established\n");
	}

	
	listen_sock = create_server();

	client_fd[0] = listen_sock;
	printf("Move in listening mode\n");
	while (1){
		listen_from_socket();
	}


	return 0;
}	

