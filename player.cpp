#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <arpa/inet.h>
using namespace std;
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[]) {

  int socket_fd;
  //int neigh_fd[2];
  int status;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  struct sockaddr_storage * neigh_inf= new struct sockaddr_storage[3];
  const char *hostname = argv[1];
  const char *port     = argv[2];
  //string msg = "connection with ringmaster";
 // char msg[512];
  if (argc != 3) {
      printf("Syntax: client %s\n", hostname);
      return EXIT_FAILURE;
  }

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

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
  status = connect(socket_fd, host_info_list->ai_addr,host_info_list->ai_addrlen);
  if(status == -1){
    cerr << "fail to connect with ringmaster" << endl;
    return EXIT_FAILURE;
  }
  status = recv(socket_fd, neigh_inf, 3*sizeof(* neigh_inf), 0);

  if(status == -1){
    cerr << "fail to receive message"<<endl;
    return EXIT_FAILURE;
  }
  for(int i =0; i < 3; i++) {
    struct sockaddr_in sa = *(struct sockaddr_in *) (neigh_inf+i);
    char ip4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
    cout << ip4 << "" << sa.sin_port << endl;
  }
  // connect with neighb
  
  freeaddrinfo(host_info_list);
  close(socket_fd);
  return EXIT_SUCCESS;

}
