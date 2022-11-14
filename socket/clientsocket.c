#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define buffersize 1024


void print_ip_opts(int sock){
  unsigned char recvopts[40] = {0};
  socklen_t rlen;
  if (getsockopt(sock, IPPROTO_IP, IP_OPTIONS, recvopts, &rlen) == -1) {
    perror("failed to get ip opts");
    return;
  }
  printf("rlen = %d\n", rlen);
  for (size_t i = 0; i < rlen; i++) {
    printf("%u: [%02x] \n", i, recvopts[i]);
  }
}

int main(int argc, char *argv[]) {

  int sock = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(5566);

//   struct sockaddr_in client_addr;
//   memset(&client_addr, 0, sizeof(client_addr));
//   client_addr.sin_family = AF_INET;
//   client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
//   client_addr.sin_port = htons(7788);
//   bind(sock, (struct sockaddr *)&client_addr, sizeof(client_addr));

  unsigned char options[16];
  memset(options, 0, sizeof(options));
  options[0] = 7;
  options[1] = 15;
  options[2] = 8;
  options[3] = 1; // Mocking as tenant footprint
  options[4] = 2;
  options[5] = 3;
  options[6] = 4;


  if (setsockopt(sock, IPPROTO_IP, IP_OPTIONS, options, sizeof(options)) ==
      -1) {
    perror("Error setting options");
    return -1;
  }
  connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  usleep(100);
  print_ip_opts(sock);

//   const char *message = "1";
//   printf("send\n");
//   send(sock, message, strlen(message) + 1, 0);

//   char *buffer = (char *)calloc(buffersize, sizeof(char));
//   printf("read\n");
//   read(sock, buffer, buffersize);
//   printf("result form server: %s\n", buffer);
  close(sock);
  return 0;
}
