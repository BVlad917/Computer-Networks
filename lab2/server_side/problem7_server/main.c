#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 7.   The client sends to the server a string and two numbers (s, I, l). The sever returns
//      to the client the substring of s starting at index I, of length l.

int main() {
    int socket_fd, new_socket_fd;
    struct sockaddr_in server, client;
    char *s, *substr;
    uint32_t I, l, s_len;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&option, sizeof(option));

    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(MYPORT);
    server.sin_family = AF_INET;
    memset(&(server.sin_zero), 0, 8);

    if (bind(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    int socket_size = sizeof(client);
    new_socket_fd = accept(socket_fd, (struct sockaddr*)&client, &socket_size);
    if (new_socket_fd < 0) {
        perror("accept");
        return 1;
    }

    if (recv(new_socket_fd, (void*)&s_len, sizeof(s_len), 0) < 0) {
        perror("receive");
        return 1;
    }
    s_len = ntohl(s_len);

    s = (char*) malloc(s_len * sizeof(char));
    if (recv(new_socket_fd, (void*)s, s_len * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    if (recv(new_socket_fd, (void*)&I, sizeof(I), 0) < 0) {
        perror("receive");
        return 1;
    }
    I = ntohl(I);

    if (recv(new_socket_fd, (void*)&l, sizeof(l), 0) < 0) {
        perror("receive");
        return 1;
    }
    l = ntohl(l);

    substr = (char*) malloc(l * sizeof(char));
    strncpy(substr, s + I, l);

    if (send(new_socket_fd, (void*)substr, l * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
