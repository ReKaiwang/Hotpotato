#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
using namespace std;
//struct _potato
//{
//	unsigned hops;
//	int * players_ID;
//};
//struct _player
//{
//	char* port_num;
//	int player_ID;
//	struct _player * neigh;
//};
//
//typedef struct _potato potato;
//typedef struct _player player;
// get the IP from socket
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
	int socket_fd;
	//unsigned num_players = argv[2];
	int num_players = atoi(argv[2]);
	//int num_hops = atoi(argv[3]);
	int* player_socketfd = new int[num_players];
	struct addrinfo host_info;
  	struct addrinfo *host_info_list;
  	struct sockaddr_storage * player_addr = new struct sockaddr_storage [num_players];
  	//char msg[256];
  	int status;
  	// initialize the host_info with 0
 	memset(&host_info, 0, sizeof(host_info));
 	host_info.ai_family   = AF_UNSPEC;
  	host_info.ai_socktype = SOCK_STREAM;
  	host_info.ai_flags    = AI_PASSIVE; // set for bind
	if((status = getaddrinfo(NULL, argv[1], & host_info, &host_info_list)) != 0){
		cerr << "getaddrinfo error:" << gai_strerror(status);
		return EXIT_FAILURE;
	}
  	socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  	if (socket_fd == -1) {
    	cerr << "Error: cannot create socket" << endl;
//    	cerr << "  (" << hostname << "," << port << ")" << endl;
    	return EXIT_FAILURE;
  	}

  	//int yes = 1;
  	//status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  	status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  	if (status == -1) {
   	 cerr << "Error: cannot bind socket" << endl;
   //	 cerr << "  (" << hostname << "," << port << ")" << endl;
    	return -1;
  	} //if

 	 status = listen(socket_fd, 100);
  	if (status == -1) {
    	cerr << "Error: cannot listen on socket" << endl; 
   // 	cerr << "  (" << hostname << "," << port << ")" << endl;
    	return -1;
  	} //if

  	cout << "Waiting for connection"<<endl;
	socklen_t addr_size =  sizeof(player_addr);
  	for(int i = 0; i < num_players; i++){
		int new_fd = accept(socket_fd, (struct sockaddr * ) (player_addr+i), & addr_size);
		if(new_fd == -1){
			cerr << "Error: cannot accept on socket"<<endl;
			return EXIT_FAILURE;
		}
		player_socketfd[i] = new_fd;
//		struct sockaddr_in sa =  * (struct sockaddr_in * ) (player_addr+i);
//		char ip4[INET_ADDRSTRLEN];
////		void * test = get_in_addr((struct sockaddr * ) (player_addr+i));
//		inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
//		cout << ip4 << ""<< sa.sin_port<<endl;
		cout << "Successful connection with " << (i+1) << " player of " << num_players << endl;
  	}
  	// send neighbor info to each player
	for(int i = 0; i < num_players; i++){
		struct sockaddr_storage *neigh_addr = new sockaddr_storage[3];
		if(i == 0) {
			neigh_addr[0] = player_addr[num_players-1];
			neigh_addr[1] = player_addr[i+1];
		} else if(i == num_players-1){
			neigh_addr[0] = player_addr[i-1];
			neigh_addr[1] = player_addr[0];
		}
		else{
			neigh_addr[0] = player_addr[i-1];
			neigh_addr[1] = player_addr[i+1];
		}
		neigh_addr[2] = player_addr[i];
		for(int i =0; i < 3; i++) {
			struct sockaddr_in sa = *(struct sockaddr_in *) (neigh_addr+i);
			char ip4[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
			cout << ip4 << "" << sa.sin_port << endl;
		}
		status = send(player_socketfd[i], neigh_addr , 3*sizeof(*(neigh_addr)), 0);
		if(status == -1 ){
			cerr << "fail to send message to player " << i << endl;
			return EXIT_FAILURE;
		}
	}

  	cout << "Ready to start the game, sending potato to player "<<endl;
	freeaddrinfo(host_info_list);
	close(socket_fd);
	for(int i = 0; i < num_players; i++){
		close(player_socketfd[i]);
	}
	return EXIT_SUCCESS;
}
