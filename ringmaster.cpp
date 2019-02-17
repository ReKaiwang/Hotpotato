#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include "potato.h"
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
	int num_hops = atoi(argv[3]);
	int* player_socketfd = new int[num_players];
	struct addrinfo host_info;
  	struct addrinfo *host_info_list;
  	struct sockaddr_storage * player_addr = new struct sockaddr_storage [num_players];
  	struct sockaddr_storage * player_listen = new struct sockaddr_storage [num_players];
  	char buf[256];
  	int status;
  	//master  fd
  	fd_set master;
	fd_set read_fds;
  	int fdmax=0; // maximum file descriptor number

  	// make a potato for the game
  	potato mypotato = {num_hops,{0},0};
  	//mypotato.players_ID = new int[num_hops];
  	// initialize the host_info with 0
 	memset(&host_info, 0, sizeof(host_info));

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

 	host_info.ai_family   = AF_INET;
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
	//FD_SET(socket_fd, &master);
	//fdmax = socket_fd;
  	cout << "Waiting for connection"<<endl;
	socklen_t addr_size =  sizeof(*player_addr);
  	for(int i = 0; i < num_players; i++){
		int new_fd = accept(socket_fd, (struct sockaddr * ) (player_addr+i), & addr_size);
		if(new_fd == -1){
			cerr << "Error: cannot accept on socket"<<endl;
			return EXIT_FAILURE;
		}
		FD_SET(new_fd, &master); // add to master set
		if (new_fd > fdmax) {    // keep track of the max
			fdmax = new_fd;
		}
		player_socketfd[i] = new_fd;
        status = recv(new_fd, player_listen+i, sizeof(* player_listen), 0);
        if(status == -1){
            cerr << "fail to receive message"<<endl;
            return EXIT_FAILURE;
        }
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
			neigh_addr[0] = player_listen[num_players-1];
			neigh_addr[1] = player_listen[i+1];
		} else if(i == num_players-1){
			neigh_addr[0] = player_listen[i-1];
			neigh_addr[1] = player_listen[0];
		}
		else{
			neigh_addr[0] = player_listen[i-1];
			neigh_addr[1] = player_listen[i+1];
		}
		neigh_addr[2] = player_listen[i];
//		for(int i =0; i < 3; i++) {
//			struct sockaddr_in sa = *(struct sockaddr_in *) (neigh_addr+i);
//			char ip4[INET_ADDRSTRLEN];
//			inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
//			cout << ip4 << "" << sa.sin_port << endl;
//		}
		status = send(player_socketfd[i], neigh_addr , 3*sizeof(*(neigh_addr)), 0);
		if(status == -1 ){
			cerr << "fail to send message to player " << i << endl;
			return EXIT_FAILURE;
		}
		status = send(player_socketfd[i],& i, sizeof(int),0);
	}
	int messg_num = 0;
	// ready for connection to neigh
//	for(int i = 0 ; i < num_players; i++) {
//	    cout<<player_socketfd[i]<<endl;
//        if ((status = recv(player_socketfd[i], buf, sizeof(buf), 0)) <= 0) {
//            if (status == 0) {
//                cout << "socket" << i << " hung up" << endl;
//            } else {
//                cerr << "receive error from " << i << endl;
//            }
//        }
//        cout<<buf<<endl;
//        memset(buf,0,sizeof(buf)/sizeof(char));
//    }
	while(1){
		read_fds = master;
		//cout<<fdmax<<endl;
		//cout<<fdmax<<endl;
		cout << "before select"<<endl;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        cout<< "now start select"<<endl;
		for(int i =0; i <= fdmax; i++){
			if(FD_ISSET(i, & read_fds)){
			    cout<<i<<endl;
				if((status = recv(i, buf, sizeof(buf),0))<=0){
					if(status==0) {
						cout << "socket" << i << " hung up" << endl;
					}
					else {
						cerr << "receive error from " << i << endl;
					}
					close(i);
					FD_CLR(i, & read_fds);
					return EXIT_FAILURE;
				}// ENDOF error check
			messg_num++;
			cout<<buf<<endl;
			memset(buf,0,sizeof(buf)/sizeof(char));
			}
		}
		if(messg_num == num_players)
			break;
	}
    for(int i = 0 ; i < num_players; i++) {
	   // cout<<player_socketfd[i]<<endl;
        if ((status = send(player_socketfd[i], "Ready to start the game", 40, 0)) <= 0) {
                if (status == 0) {
                   cout << "socket" << i << " hung up" << endl;}
                else {
                cerr << "receive error from " << i << endl;
            }
        }

    }
	srand( (unsigned int) time(NULL));
    int first_player = rand()% num_players;
	cout << "Ready to start the game, sending potato to player "<<(first_player+1)<<endl;
    if((status = send(player_socketfd[first_player],& mypotato, sizeof(potato),0))==-1){
    	cerr<<"fail to send potato to "<<(first_player+1)<<endl;
		return EXIT_FAILURE;
    }
    cout << "wait for potato"<<endl;
    while(1){
		read_fds = master;
		//cout<<fdmax<<endl;
		//cout<<fdmax<<endl;
		//cout << "before select"<<endl;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
		//cout<< "now start select"<<endl;
		for(int i =0; i <= fdmax; i++){
			if(FD_ISSET(i, & read_fds)){
				cout<<i<<endl;
				if((status = recv(i, &mypotato, sizeof(mypotato),0))!= sizeof(mypotato)){
					if(status==0) {
						cout << "socket" << i << " hung up" << endl;
					}
					else if(status <0 ){
						cerr << "receive error from " << i << endl;
					}
					else
						cout << "not potato information"<<endl;
					close(i);
					FD_CLR(i, & read_fds);
					return EXIT_FAILURE;
				}// ENDOF error check
				else{
					cout << "I'm it!"<<endl;
					return EXIT_SUCCESS;
				}
			}
		}
    }
	freeaddrinfo(host_info_list);
	close(socket_fd);
	for(int i = 0; i < num_players; i++){
		close(player_socketfd[i]);
	}
	return EXIT_SUCCESS;
}
