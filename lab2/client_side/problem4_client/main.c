#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777

// 4.   The client send to the server two sorted array of chars. The server will merge sort the two
//      arrays and return the result to the client.

int main() {
    int socket_fd;
    struct sockaddr_in server;
    char* s1 = "acegi";
    char* s2 = "bdfhjklmn";

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("192.168.1.103");
    memset(&(server.sin_zero), 0, 8);
    server.sin_port = htons(MYPORT);
    server.sin_family = AF_INET;

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }

    uint32_t len_s1 = strlen(s1), len_s2 = strlen(s2), len_s3;

    len_s1 = htonl(len_s1);
    if (send(socket_fd, (void*)&len_s1, sizeof(len_s1), 0) < 0) {
        perror("send");
        return 1;
    }
    len_s1 = ntohl(len_s1);

    if (send(socket_fd, (void*)s1, len_s1 * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    len_s2 = htonl(len_s2);
    if (send(socket_fd, (void*)&len_s2, sizeof(len_s2), 0) < 0) {
        perror("send");
        return 1;
    }
    len_s2 = ntohl(len_s2);

    if (send(socket_fd, (void*)s2, len_s2 * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    if (recv(socket_fd, (void*)&len_s3, sizeof(len_s3), 0) < 0) {
        perror("receive");
        return 1;
    }
    len_s3 = ntohl(len_s3);

    char* s3 = (char*) malloc(len_s3 * sizeof(char));

    if (recv(socket_fd, (void*)s3, len_s3 * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    close(socket_fd);
    printf("%s\n", s3);

    return 0;
}
