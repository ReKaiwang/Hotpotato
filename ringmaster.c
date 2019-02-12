#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
struct _potato
{
	unsigned hops;
	int * player_ID;
};
typedef struct _potato potato;
int main(int argc, char *argv[]) {
	int * socket_fd;
	struct addrinfo host_info;
  	struct addrinfo *host_info_list;

}