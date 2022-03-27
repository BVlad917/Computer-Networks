#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <arpa/inet.h>

#define MYPORT 8080
#define BACKLOG 5

// 1.   A client sends to the server an array of numbers. Server returns the sum of the numbers.

int main() {
    int socket_fd, new_socket_fd;
    struct sockaddr_in client, server;
    uint32_t alen, sum;
    int bytes_sent, bytes_read;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*) &option, sizeof(option));

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(MYPORT);
    server.sin_family = AF_INET;
    memset(&(server.sin_zero), 0, 8);

    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    int socket_size = sizeof(struct sockaddr_in);
    new_socket_fd = accept(socket_fd, (struct sockaddr *)&client, &socket_size);
    if (new_socket_fd < 0) {
        perror("accept");
        return 1;
    }

//    struct sockaddr_in save1;
//    int size_save1 = sizeof(save1);
//    getsockname(new_socket_fd, (struct sockaddr*)&save1, &size_save1);
//    printf("getsockname: %s:%d\n", inet_ntoa(save1.sin_addr), save1.sin_port);
//
//    struct sockaddr_in save2;
//    int size_save2 = sizeof(save2);
//    getpeername(new_socket_fd, (struct sockaddr*)&save2, &size_save2);
//    printf("getpeername: %s:%d\n", inet_ntoa(save2.sin_addr), save2.sin_port);
//    output: getsockname: 192.168.1.102:36895  -> 192.168.1.102 is MY IP address
//            getpeername: 192.168.1.105:16049  -> 192.168.1.105 is HIS IP address

    bytes_read = recv(new_socket_fd, (void *)&alen, sizeof(alen), 0);
    if (bytes_read < 0) {
        perror("receive array length");
        return 1;
    }

    alen = ntohl(alen);
    uint32_t* a = (uint32_t *) malloc(alen * sizeof(uint32_t));

    bytes_read = recv(new_socket_fd, (void *)a, (int)(alen * sizeof(uint32_t)), 0);
    if (bytes_read < 0) {
        perror("read");
        return 1;
    }

    sum = 0;
    for (int i = 0; i < alen; i++) {
        a[i] = ntohl(a[i]);
        sum += a[i];
    }

    sum = htonl(sum);
    bytes_sent = send(new_socket_fd, (void *)&sum, sizeof(sum), 0);
    if (bytes_sent < 0) {
        perror("send (sum array)");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
