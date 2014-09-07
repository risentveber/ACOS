#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

#define COUNT_ITEMS 10000
#define SIZE_BUFF 30000
#define PORT 32000

struct timeval tv1,tv2,dtv;

/*struct timezone 
{
	int tz_minuteswest; 
	int tz_dsttime;         
};*/

struct timezone tz;

void time_start()
{
	gettimeofday(&tv1, &tz);
}

long time_stop()
{
	gettimeofday(&tv2, &tz);
	dtv.tv_sec= tv2.tv_sec -tv1.tv_sec;
	dtv.tv_usec=tv2.tv_usec-tv1.tv_usec;
	if(dtv.tv_usec<0)
	{
		dtv.tv_sec--;
		dtv.tv_usec+=1000000;
	}
	return dtv.tv_sec*1000+dtv.tv_usec/1000;
}


