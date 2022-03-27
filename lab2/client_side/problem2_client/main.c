#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define MYPORT 7777

// 2.   A client sends to the server a string. The server returns the count of spaces in the string.

int main() {
    int socket_fd, bytes_sent, bytes_read;
    struct sockaddr_in server;
    char* s = "how much wood can a woodchuck chuck";
    uint32_t spaces_cnt;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    server.sin_family = AF_INET;
    memset(&(server.sin_zero), 0, 8);
    server.sin_port = htons(MYPORT);
    server.sin_addr.s_addr = inet_addr("192.168.1.103");

    if (connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }

    uint32_t len = strlen(s);
    len = htonl(len);

    bytes_sent = send(socket_fd, (void *)&len, sizeof(len), 0);
    if (bytes_sent < 0) {
        perror("send (string length)");
        return 1;
    }

    len = ntohl(len);
    bytes_sent = send(socket_fd, (void *)s, len * sizeof(char), 0);
    if (bytes_sent < 0) {
        perror("send (string)");
        return 1;
    }

    bytes_read = recv(socket_fd, (void *)&spaces_cnt, sizeof(spaces_cnt), 0);
    if (bytes_read < 0) {
        perror("receive (spaces count)");
        return 1;
    }
    spaces_cnt = ntohl(spaces_cnt);

    close(socket_fd);

    printf("Number of spaces = %d\n", spaces_cnt);

    return 0;
}
