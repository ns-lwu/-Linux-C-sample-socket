#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define buffersize 1024
//socklen_t optlen = sizeof(getopt);
void get_ip_opts(int sock) {
  // Get ip options set on sock
  // Result: only the ip options we set are retrieved
  unsigned char getopt[40] = {0};
  socklen_t optlen = 0;
  if (getsockopt(sock, IPPROTO_IP, IP_OPTIONS, (char *)&getopt, &optlen) ==
      -1) {
    perror("Error get setting options\n");
  }
  printf("oplen %d", optlen);
  printf("====opt start in dec====\n");

  for (size_t i = 0; i < optlen; i++) {
    printf("%d ", getopt[i]);
  }
  printf("\n====opt end====\n");
}

int main(int argc, char *argv[]){
 
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    // Ref https://stackoverflow.com/a/39298615/4123703
    unsigned char options[16];
    memset(options, 0, sizeof(options));
    options = {
        7,    // option type 30 (experimental)
        15,    // option length
        4,   // option data
        0,
    };
    // unsigned char options[40] = {1};
    if (setsockopt(sock, IPPROTO_IP, IP_OPTIONS, (char *)&options, sizeof(options))== -1) {
        perror("Error setting options");
        close(sock);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    serv_addr.sin_port = htons(5566); 
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    const char *message = "1";
    // printf("send\n");
    // send(sock, message, strlen(message)+1, 0) ;

    // char *buffer = (char*) calloc(buffersize, sizeof(char)) ;
    // printf("read\n");
    // read(sock, buffer, buffersize);
    // printf("result form server: %s\n", buffer);

    get_ip_opts(sock);

    free(buffer);
    close(sock);
    return 0;
}
