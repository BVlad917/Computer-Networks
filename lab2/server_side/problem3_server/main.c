#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 3.   A client sends to the server a string. The server returns the reversed string to
//      the client (characters from the end to begging)

int main() {
    int socket_fd, new_socket_fd, bytes_sent, bytes_received;
    struct sockaddr_in client, server;
    uint32_t len;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }
    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(option));

    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(MYPORT);
    server.sin_family = AF_INET;
    memset(&(server.sin_zero), 0, 8);

    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
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
        perror("accept");
        return 1;
    }

    bytes_received = recv(new_socket_fd, (void *)&len, sizeof(len), 0);
    if (bytes_received < 0) {
        perror("receive");
        return 1;
    }

    len = ntohl(len);
    char* s = (char*) malloc(len * sizeof(char));

    bytes_received = recv(new_socket_fd, s, len * sizeof(char), 0);
    if (bytes_received < 0) {
        perror("receive");
        return 1;
    }

    char aux;
    for (int i = 0; i < len / 2; i++) {
        aux = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = aux;
    }

    bytes_sent = send(new_socket_fd, s, len * sizeof(char), 0);
    if (bytes_sent < 0) {
        perror("send");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
