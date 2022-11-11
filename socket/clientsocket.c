#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define buffersize 1024

int main(int argc, char *argv[]){
 
    //int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP); doesn't help to read IP_OPTIONS as well..
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    // Ref https://stackoverflow.com/a/39298615/4123703
    unsigned char options[] = {
        30,    // option type 30 (experimental)
        14,    // option length
        1,2,3,4,5,6,7,8,9,10,11,12,   // option data
        1,     // option type 1 (no-op, no length field)
        1      // option type 1 (no-op, no length field)
    };
    // unsigned char options[40] = {1};
    if (setsockopt(sock, IPPROTO_IP, IP_OPTIONS, (char *)&options, sizeof(options))== -1) {
        perror("Error setting options");
        close(sock);
    }
    // // This set doesn't support SOCK_STREAM, refer to
    // // https://man7.org/linux/man-pages/man7/ip.7.html section IP_RECVOPTS
    // int option = 1;
    // if (setsockopt(sock, IPPROTO_IP, IP_RECVOPTS, (char *)&option, sizeof(option)) == -1) {
    //     perror("Error setting IP_RECVOPTS");
    //     close(sock);
    //  exit(1);
    // }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(5566);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    const char *message = "11";
    printf("send\n");
    send(sock, message, strlen(message)+1, 0);
    
    printf("send done\n");
//    { // Since IP_RECVOPTS isn't supported, `cmhdr` is always NULL
//       // just put it here to prove that it's not supported
//       struct sockaddr_in sin_recv;
//       char mes[1500];
//       struct msghdr mhdr;
//       struct iovec iov;
//       struct cmsghdr *cmhdr;
//       char control[1000];
//       int len;
//       unsigned char *recv_options;
//       unsigned int i;

//       mhdr.msg_name = &sin_recv;
//       mhdr.msg_namelen = sizeof(sin_recv);
//       mhdr.msg_iov = &iov;
//       mhdr.msg_iovlen = 1;
//       mhdr.msg_control = &control;
//       mhdr.msg_controllen = sizeof(control);
//       iov.iov_base = mes;
//       iov.iov_len = sizeof(mes);
//       if ((len = recvmsg(sock, &mhdr, 0)) == -1) {
//         perror("Error receiving");
//     } else {
//         cmhdr = CMSG_FIRSTHDR(&mhdr); // In SOCK_STREAM, `cmhdr` is always NULL
//         printf("cmhdr address %p\n", cmhdr);
//         while (cmhdr) {
//             if (cmhdr->cmsg_level == IPPROTO_IP &&
//                     cmhdr->cmsg_type == IP_RECVOPTS) {
//                 recv_options = CMSG_DATA(cmhdr);
//                 printf("options: ");
//                 for (i=0;i<cmhdr->cmsg_len-sizeof(struct cmsghdr);i++) {
//                     printf("%02x ", recv_options[i]);
//                 }
//                 printf("\n");
//             }
//             cmhdr = CMSG_NXTHDR(&mhdr, cmhdr);
//         }
//     }
//     }

    char *buffer = (char*) calloc(buffersize, sizeof(char)) ;
    printf("read\n");
    read(sock, buffer, buffersize);

    // Get ip options set on sock
    // Result: 
    struct ip_opts getopt = {0};
    socklen_t optlen = sizeof(getopt);
    if (getsockopt(sock, IPPROTO_IP, IP_OPTIONS, (char *)&getopt, &optlen)== -1) {
        perror("Error get setting options");
        close(sock);
    }
    printf("retrieved options dst %u\n", getopt.ip_dst.s_addr);
    for(int i = 0; i < 4; i++){
        printf("retrieved options dst %u\n", (getopt.ip_dst.s_addr >> (i*2)) & 0xFF );
    }
    printf("====ip_opts start ====\n");
    for(size_t i = 0; i < sizeof(getopt.ip_opts); i++){
        printf("%d ", getopt.ip_opts[i]);
    }
    printf("\n====ip_ipts end====\n");

    printf("result form server: %s\n", buffer);

    free(buffer);
    close(sock);
    return 0;
}
