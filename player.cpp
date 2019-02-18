#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include "potato.h"
using namespace std;
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[]) {
  //socket_fd use to connect with server and listen_fd use to listen
  int socket_fd, listen_fd;
  //int neighb_fd[2];
  int neigh_fd[2]; //
  int playernum[2]; // player number of client
  int status; //check error status
  struct addrinfo host_info, listen_info; // hits
  struct addrinfo *host_info_list, *listen_info_list;
  struct sockaddr_storage * neigh_inf= new struct sockaddr_storage[3]; // store neighbor listen socket
  struct sockaddr_storage * neigh_add= new struct sockaddr_storage[2]; //
  const char *hostname = argv[1];
  const char *port     = argv[2];
  const char * end = "endgame";
  //bound port address
  struct sockaddr *listen_addr = new struct sockaddr;
  //message to server
  char msg[256];

  // make an empty potato
  potato mypotato;

  //set for select
  fd_set master;
  fd_set read_fds;
  int fdmax=0; // maximum file descriptor number
  FD_ZERO(&master);    // clear the master and temp sets
  FD_ZERO(&read_fds);

  if (argc != 3) {
      printf("Syntax: client %s\n", hostname);
      return EXIT_FAILURE;
  }

  memset(&host_info, 0, sizeof(host_info));
  memset(&listen_info,0, sizeof(listen_info));
  host_info.ai_family   = AF_INET;
  host_info.ai_socktype = SOCK_STREAM;

  //set for listen port
  listen_info.ai_family   = AF_INET;
  listen_info.ai_socktype = SOCK_STREAM;
  listen_info.ai_flags    = AI_PASSIVE;
  //get the address for player connections
  status = getaddrinfo(NULL,port, & listen_info, &listen_info_list);
    if (status != 0) {
        perror("Error: cannot get address info for listen\n");
        return EXIT_FAILURE;
    }

  //get the address for connection with ringmaster
  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    perror("Error: cannot get address info for host\n");
    return EXIT_FAILURE;
  } //if

  socket_fd  = socket(host_info_list->ai_family,
                      host_info_list->ai_socktype,
                      host_info_list->ai_protocol);
  if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return EXIT_FAILURE;
  }

  fdmax = socket_fd;
  FD_SET(socket_fd, &master);

  // make a socket for listenning
  listen_fd = socket(listen_info_list->ai_family,
                     listen_info_list->ai_socktype,
                     listen_info_list->ai_protocol);

  if (listen_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return EXIT_FAILURE;
  }

//  FD_SET(listen_fd, &master);
//  if(listen_fd > fdmax)
//      fdmax = listen_fd;

  // make listen port arbitary
  ( (struct sockaddr_in *)listen_info_list->ai_addr)->sin_port = 0;

  // bind to port
  status = bind(listen_fd, listen_info_list->ai_addr, listen_info_list->ai_addrlen);
  if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        //	 cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    }

    //begin to listen
    status = listen(listen_fd, 100);
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl;
        // 	cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if
    //get the listen socket address and port
    status  = getsockname(listen_fd,listen_addr,& listen_info_list->ai_addrlen);
    if(status == -1){
        cerr << "fail to get listen socket name" << endl;
        return EXIT_FAILURE;
    }

  //conect with ringmster
  status = connect(socket_fd, host_info_list->ai_addr,host_info_list->ai_addrlen);
  if(status == -1){
    cerr << "fail to connect with ringmaster" << endl;
    return EXIT_FAILURE;
  }
  // send address and port information to ringmaster
  status = send(socket_fd,listen_addr,(size_t)listen_info_list->ai_addrlen,0 );
  // receive neighbour information from ringmaster
  status = recv(socket_fd, neigh_inf, 3*sizeof(* neigh_inf), 0);
  if(status == -1){
    cerr << "fail to receive message"<<endl;
    return EXIT_FAILURE;
  }

  //receive player number from ringmaster
  status = recv(socket_fd,playernum , sizeof(playernum), 0);
    if(status == -1){
        cerr << "fail to receive message"<<endl;
        return EXIT_FAILURE;
    }
  cout <<"Connected as player "<<playernum[0]<<" of "<<playernum[1]<<" total players"<<endl;
//  for(int i =0; i < 3; i++) {
//    struct sockaddr_in sa = *(struct sockaddr_in *) (neigh_inf+i);
//    char ip4[INET_ADDRSTRLEN];
//    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
//    cout << ip4 << "" << sa.sin_port << endl;
//  }

  // connect with neighb
  neigh_fd[0] = socket(listen_info_list->ai_family,
                       listen_info_list->ai_socktype,
                       listen_info_list->ai_protocol);
  socklen_t addr_size =  sizeof(*listen_addr);
  if(playernum[0] == 0){
      status = connect(neigh_fd[0],(struct sockaddr * )neigh_inf, addr_size);
      if(status == -1){
          cerr << "fail to connect with neigh" << endl;
          return EXIT_FAILURE;
      }
      neigh_fd[1] = accept(listen_fd, (struct sockaddr * ) (neigh_add+1), &addr_size);
      if(neigh_fd[1] == -1){
          cerr << "Error: cannot accept on socket"<<endl;
          return EXIT_FAILURE;
      }
  }
  else{
      neigh_fd[1] = accept(listen_fd, (struct sockaddr * ) (neigh_add+1), &addr_size);
      if(neigh_fd[1] == -1){
          cerr << "Error: cannot accept on socket"<<endl;
          return EXIT_FAILURE;
      }
      status = connect(neigh_fd[0],(struct sockaddr * )neigh_inf, addr_size);
      if(status == -1){
          cerr << "fail to connect with neigh" << endl;
          return EXIT_FAILURE;
      }

  }
    FD_SET(neigh_fd[0], &master);
    if(neigh_fd[0] > fdmax)
        fdmax = neigh_fd[0];
    FD_SET(neigh_fd[1], &master);
    if(neigh_fd[1] > fdmax)
        fdmax = neigh_fd[1];

    status = send(socket_fd, "ready to play game", 20, 0);
      if (status == -1) {
          cerr << "fail to send listen message" << endl;
          return EXIT_FAILURE;
      }

      memset(msg,0,sizeof(msg)/sizeof(char));
      status = recv(socket_fd,msg,sizeof(msg)/ sizeof(char),0);
        if (status == -1) {
        cerr << "fail to receive ready message" << endl;
        return EXIT_FAILURE;
        }

      //wait for potato
    srand( (unsigned int) time(NULL)+playernum[0]);
      while(1){
          read_fds = master;
          if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
              perror("select");
              exit(4);
          }
          //cout<<"after select"<<endl;
          for(int i =0; i <= fdmax; i++){
              if(FD_ISSET(i, & read_fds)){
                if((status = recv(i, &mypotato, sizeof(mypotato),0))!= sizeof(mypotato))
                  {
                    if(status == 0)
                        cout << "socket" << i << " hung up" << endl;
                    else if(status <0)
                        cerr << "receive error from " << i << endl;
                    else {
                        if(status == sizeof(end)/ sizeof(char))
                        {
                            freeaddrinfo(host_info_list);
                            freeaddrinfo(listen_info_list);
                            close(socket_fd);
                            close(listen_fd);
                            for(int j=0; j < 2; j++){
                                close(neigh_fd[j]);
                            }
                            return EXIT_SUCCESS;
                        }
                        else
                        cout << "not potato message!" << endl;
                    }
                    close(i);
                    FD_CLR(i, & read_fds);
                    return EXIT_FAILURE;
                  }
                else{
                    mypotato.hops--;
                    mypotato.players_ID[mypotato.already] = playernum[0];
                    mypotato.already++;
                    //cout << mypotato.players_ID[mypotato.already-1]<<endl;
                    if(mypotato.hops <= 0){
                        if((status = send(socket_fd, &mypotato, sizeof(mypotato),0 ))==-1){
                            cerr<<"fail to return potota to ringmaster";
                            return EXIT_FAILURE;
                        }
                    }else{
                        int next_player = rand()%2;
                        int next_num ;
                        if(next_player == 0)
                            next_num = playernum[0] - 1;
                        else
                            next_num = playernum[0] +1 ;
                        if(next_num < 0)
                            next_num = playernum[1] - 1;
                        else if (next_num >= playernum[1])
                            next_num = 0;
                        cout << "Sending potato to " << next_num<<endl;
                        if((status = send(neigh_fd[next_player],&mypotato, sizeof(mypotato),0 ))==-1){
                            cerr<<"fail to return potota to ringmaster";
                            return EXIT_FAILURE;
                        }
                    }
                }
              }// end isset
          }// end iterate
      }


  freeaddrinfo(host_info_list);
  freeaddrinfo(listen_info_list);
  close(socket_fd);
  return EXIT_SUCCESS;

}
