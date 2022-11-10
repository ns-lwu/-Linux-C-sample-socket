#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define buffersize 1024

// set to 1, IP_OPTIONS would be set on server sock
// set to 0, IP_OPTIONS would be set on client sock
#define SET_IPOPT_ON_SERVER_SOCK 1

void sig_fork(int signo) {
    int stat;
    waitpid(0, &stat, WNOHANG);
    return;
}

int main(){
    printf("pid %d\n", getpid()) ;
    signal(SIGCHLD, sig_fork); 
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    printf("serv_sock fd %d\n", serv_sock);

#if SET_IPOPT_ON_SERVER_SOCK == 1
    // Ref https://stackoverflow.com/a/39298615/4123703
    unsigned char options[] = {
        30,    // option type 30 (experimental)
        14,    // option length
        11,12,13,14,15,16,17,18,19,20,21,22,   // option data
        1,     // option type 1 (no-op, no length field)
        1      // option type 1 (no-op, no length field)
    };
    // unsigned char options[40] = {1};
    printf("option size %lu", sizeof(options));
    if (setsockopt(serv_sock, IPPROTO_IP, IP_OPTIONS, (char *)&options,
                   sizeof(options)) == -1) {
      perror("Error setting options on server sock");
      close(serv_sock);
      return -1;
    }
#endif

    struct timeval timeout;
    timeout.tv_sec = 5; // sec
    timeout.tv_usec = 0; // ms

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    serv_addr.sin_port = htons(5566);  
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(serv_sock, 20);

    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    while (1) {
      int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

#if SET_IPOPT_ON_SERVER_SOCK == 0
      // //  Ref https://stackoverflow.com/a/39298615/4123703
      unsigned char options[] = {
          30, // option type 30 (experimental)
          14, // option length
          11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, // option data
          1, // option type 1 (no-op, no length field)
          1  // option type 1 (no-op, no length field)
      };
      printf("size %lu", sizeof(options));
      if (setsockopt(clnt_sock, IPPROTO_IP, IP_OPTIONS, (char *)&options,
                     sizeof(options)) == -1) {
        perror("Error setting after socket created");
        // printf("%d", errno);
        close(serv_sock);
        close(clnt_sock);
        return -1;
      }
#endif
      // set recv and send timeout
      printf("accepted\n");
      if( setsockopt (clnt_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 )
        printf( "setsockopt fail\n" );
      if( setsockopt (clnt_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0 )
        printf( "setsockopt fail\n" ) ;

      int pid = fork();
      if (pid == -1) {
        printf( "ERROR pid\n" );
      } 
       
      else if (pid == 0) {
        printf("pid %d\n", getpid()) ;
        char *buffer = (char*) calloc(buffersize, sizeof(char));
        recv(clnt_sock, buffer, buffersize, 0) ;

        unsigned char recvopts[40] = {0};
        if (getsockopt(clnt_sock, IPPROTO_IP, IP_OPTIONS, (void *)&recvopts,
                      (socklen_t *)sizeof(recvopts)) == -1) {
          perror("failed to get ip opts");
        }
        for (int i = 0; i < 10; i++) {
          printf(" [%x] ", recvopts[i]);
        }

        printf("Server receive:%s\n", buffer) ;
        int total = (int)(atoi(buffer)*atoi(buffer)) ;
        printf("Server return num^2:%d\n", total) ;
        char *returnvalue = (char*) calloc(buffersize, sizeof(char));
        sprintf( returnvalue, "result:%d", total ) ;

        send(clnt_sock, returnvalue, buffersize, 0) ;
        close(clnt_sock) ;
        close(serv_sock) ;
        return 0 ;
      } 
      else {
        close(clnt_sock);
      }

    }    
       
    close(serv_sock);
    return 0;
}
