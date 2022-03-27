#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777

// 3.   A client sends to the server a string. The server returns the reversed string to
//      the client (characters from the end to begging)

int main() {
    int socket_fd, bytes_sent, bytes_received;
    struct sockaddr_in server;
    uint32_t len;
    char* s = "invert this";

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    server.sin_port = htons(MYPORT);
    server.sin_family = AF_INET;
    memset(&(server.sin_zero), 0, 8);
    server.sin_addr.s_addr = inet_addr("192.168.1.103");

    if (connect(socket_fd, (struct sockaddr *)&server, sizeof(server))) {
        perror("connect");
        return 1;
    }

    len = strlen(s);
    len = htonl(len);

    bytes_sent = send(socket_fd, (void*)&len, sizeof(len), 0);
    if (bytes_sent < 0) {
        perror("send");
        return 1;
    }

    len = ntohl(len);
    bytes_sent = send(socket_fd, (void *)s,  len * sizeof(char), 0);
    if (bytes_sent < 0) {
        perror("send");
        return 1;
    }

    char* inverse = (char *)malloc(len * sizeof(char));
    bytes_received = recv(socket_fd, (void *)inverse, len * sizeof(char), 0);
    if (bytes_received < 0) {
        perror("receive");
        return 1;
    }

    close(socket_fd);
    printf("%s\n", inverse);

    return 0;
}
