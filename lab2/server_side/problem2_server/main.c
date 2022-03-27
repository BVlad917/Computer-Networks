#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 2.   A client sends to the server a string. The server returns the count of spaces in the string.

int main() {
    int socket_fd, new_socket_fd, bytes_read, bytes_sent;
    struct sockaddr_in client, server;
    uint32_t len, spaces_cnt;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("initial socket");
        return 1;
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    server.sin_family = AF_INET;
    server.sin_port = htons(MYPORT);
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    memset(&(server.sin_zero), 0, 8);

    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    int socket_size = sizeof(client);
    new_socket_fd = accept(socket_fd, (struct sockaddr *)&client, &socket_size);

    if (new_socket_fd < 0) {
        perror("connection socket");
        return 1;
    }

    bytes_read = recv(new_socket_fd, (void *)&len, sizeof(len), 0);
    if (bytes_read < 0) {
        perror("receive (string length)");
        return 1;
    }

    len = ntohl(len);

    char* s = (char *)malloc(len * sizeof(char));
    bytes_read = recv(new_socket_fd, (void *)s, len * sizeof(char), 0);
    if (bytes_read < 0) {
        perror("receive (string)");
        return 1;
    }

    spaces_cnt = 0;
    for (int i = 0; i < len; i++) {
        if (s[i] == ' ') {
            spaces_cnt += 1;
        }
    }

    spaces_cnt = htonl(spaces_cnt);
    bytes_sent = send(new_socket_fd, (void *)&spaces_cnt, sizeof(spaces_cnt), 0);
    if (bytes_sent < 0) {
        perror("send (spaces count)");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
