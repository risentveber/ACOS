#include "time_starter.h"

int main(int argc, char** argv)
{
   int sockfd;
   struct sockaddr_in servaddr, cliaddr;
   socklen_t len;
   char mesg[SIZE_BUFF];

   sockfd = socket(AF_INET,SOCK_DGRAM,0);
 
   int child = fork();
   if (child){
      memset(&servaddr, 0, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = INADDR_ANY;
      servaddr.sin_port = htons(PORT);
      bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
      perror("end");
      recvfrom(sockfd, mesg, SIZE_BUFF, 0, (struct sockaddr *)&cliaddr, &len);   

      time_start();     
      for (int i = 0; i < COUNT_ITEMS-1; ++i){
         len = sizeof(cliaddr);
         recvfrom(sockfd, mesg, SIZE_BUFF, 0, (struct sockaddr *)&cliaddr, &len);
      }

      long int time = time_stop();

      long int size = COUNT_ITEMS*SIZE_BUFF/(1024*1024.0);
      printf("Size: %ld Mb, Time: %ld ms, Speed: %ld Mb/s\n", size, time, 1000*size/time);

      wait(NULL);
      close(sockfd);
   } else {
      int sockfd;
      struct sockaddr_in servaddr;
      char sendline[SIZE_BUFF];

      sockfd = socket(AF_INET,SOCK_DGRAM,0);
      memset(&servaddr, 0, sizeof(servaddr));

      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
      servaddr.sin_port = htons(PORT);
      perror("ddddd");
      for (int i = 0; i < COUNT_ITEMS; ++i)
         sendto(sockfd, sendline, SIZE_BUFF, 0 ,(struct sockaddr *)&servaddr, sizeof(servaddr));

      close(sockfd);
      return 0;
   }
}




