#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777

// 7.   The client sends to the server a string and two numbers (s, I, l). The sever returns
//      to the client the substring of s starting at index I, of length l.

int main() {
    int socket_fd;
    struct sockaddr_in server;
    char *s = "ana are mere multe", *substr;
    uint32_t I = 4, l = 8, s_len = strlen(s);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
    }

    server.sin_port = htons(MYPORT);
    memset(&(server.sin_zero), 0, 8);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.103");

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }

    s_len = htonl(s_len);
    if (send(socket_fd, (void*)&s_len, sizeof(s_len), 0) < 0) {
        perror("send");
        return 1;
    }

    s_len = ntohl(s_len);
    if (send(socket_fd, (void*)s, s_len * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    I = htonl(I);
    l = htonl(l);
    if (send(socket_fd, (void*)&I, sizeof(I), 0) < 0) {
        perror("send");
        return 1;
    }
    if (send(socket_fd, (void*)&l, sizeof(l), 0) < 0) {
        perror("send");
        return 1;
    }

    l = ntohl(l);
    substr = (char*) malloc(l * sizeof(char));
    if (recv(socket_fd, (void*)substr, l * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    close(socket_fd);

    printf("%s\n", substr);

    return 0;
}
