#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>


void sig_fork(int signo) {
    int stat;
    waitpid(0, &stat, WNOHANG);
    return;
}

// void print_ip_opts(int sock) {
//   // Get ip options set on sock
//   // Result: only the ip options we set are retrieved
//   char recvopts[40] = {0};
//   socklen_t rlen = 0;
//     if (getsockopt(sock, IPPROTO_IP, IP_OPTIONS, recvopts, &rlen) == -1) {
//       perror("failed to get ip opts");
//       return;
//     }
//   printf("oplen %d", rlen);
//   printf("====opt start in dec====\n");
//   for (size_t i = 0; i < rlen; i++) {
//     printf("%d: %02x ",i , recvopts[i]);
//   }
//   printf("\n====opt end====\n");
// }

void print_ip_opts(int sock){
  unsigned char recvopts[40] = {0};
  socklen_t rlen;
  if (getsockopt(sock, IPPROTO_IP, IP_OPTIONS, recvopts, &rlen) == -1) {
    perror("failed to get ip opts");
    return;
  }
  printf("rlen = %d\n", rlen);
  for (size_t i = 0; i < rlen; i++) {
    printf("%u: [%02x] \n", recvopts[i]);
  }
}
int main() {
  printf("pid %d\n", getpid()) ;
  signal(SIGCHLD, sig_fork); 
  int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  printf("serv_sock fd %d\n", serv_sock);

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(5566);

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
    perror("bind failed");
    close(serv_sock);
    return -1;
  };
  listen(serv_sock, 20);

  struct sockaddr_in clnt_addr;
  socklen_t clnt_addr_size = sizeof(clnt_addr);

  while (1) {
    int clnt_sock =
        accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    // set recv and send timeout
    printf("accepted\n");

    // print_ip_opts(clnt_sock);
    unsigned char recvopts[40] = {0};
    socklen_t rlen;
    if (getsockopt(clnt_sock, IPPROTO_IP, IP_OPTIONS, recvopts, &rlen) == -1) {
      perror("failed to get ip opts");
      close(clnt_sock);
      close(serv_sock);
      return -1;
    }
    printf("rlen = %d\n", rlen);
    for (int i = 0; i < rlen; i++) {
      printf("[%02x] \n", recvopts[i]);
    }

    usleep(100);
  }

  close(serv_sock);
  return 0;
}
