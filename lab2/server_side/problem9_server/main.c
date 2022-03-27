#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 9. The client sends to the server two arrays of numbers. The server returns to the client a list
//    of numbers that are present in the first arrays but not in the second.

int main() {
    int socket_fd, new_socket_fd, i, j;
    struct sockaddr_in server, client;
    uint32_t *a, *b, *c, a_len, b_len, c_len;

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

    if (recv(new_socket_fd, (void*)&a_len, sizeof(a_len), 0) < 0) {
        perror("receive");
        return 1;
    }
    a_len = ntohl(a_len);

    a = (uint32_t*) malloc(a_len * sizeof(uint32_t));
    if (recv(new_socket_fd, (void*)a, a_len * sizeof(uint32_t), 0) < 0) {
        perror("receive");
        return 1;
    }
    for (i = 0; i < a_len; i++) {
        a[i] = ntohl(a[i]);
    }

    if (recv(new_socket_fd, (void*)&b_len, sizeof(b_len), 0) < 0) {
        perror("receive");
        return 1;
    }
    b_len = ntohl(b_len);

    b = (uint32_t*) malloc(b_len * sizeof(uint32_t));
    if (recv(new_socket_fd, (void*)b, b_len * sizeof(uint32_t), 0) < 0) {
        perror("receive");
        return 1;
    }
    for (i = 0; i < b_len; i++) {
        b[i] = ntohl(b[i]);
    }

    c_len = 0;
    for (i = 0; i < a_len; i++) {
        for (j = 0; j < b_len; j++) {
            if (a[i] == b[j]) {
                break;
            }
        }
        if (j == b_len) {
            c_len += 1;
        }
    }

    c = (uint32_t*) malloc(c_len * sizeof(uint32_t));
    int index = 0;
    for (i = 0; i < a_len; i++) {
        for (j = 0; j < b_len; j++) {
            if (a[i] == b[j]) {
                break;
            }
        }
        if (j == b_len) {
            c[index] = a[i];
            index += 1;
        }
    }

    c_len = ntohl(c_len);
    if (send(new_socket_fd, (void*)&c_len, sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    c_len = htonl(c_len);
    for (i = 0; i < c_len; i++) {
        c[i] = htonl(c[i]);
    }

    if (send(new_socket_fd, (void*)c, c_len * sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
