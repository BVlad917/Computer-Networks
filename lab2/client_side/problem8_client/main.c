#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777

// 8.   The client sends to the server two arrays of integers. The server returns an arrays
//      containing the common numbers found in both arrays.

int main() {
    int socket_fd;
    struct sockaddr_in server;
    uint32_t a[] = {1, 2, 7, 9, 11};
    uint32_t b[] = {7, 13, 22, 11};
    uint32_t a_len = 5, b_len = 4, c_len, *c;

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

    a_len = htonl(a_len);
    if (send(socket_fd, (void*)&a_len, sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    a_len = ntohl(a_len);
    for (int i = 0; i < a_len; i++) {
        a[i] = htonl(a[i]);
    }

    if (send(socket_fd, (void*)a, a_len * sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    b_len = htonl(b_len);
    if (send(socket_fd, (void*)&b_len, sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    b_len = ntohl(b_len);
    for (int i = 0; i < b_len; i++) {
        b[i] = htonl(b[i]);
    }

    if (send(socket_fd, (void*)b, b_len * sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    if (recv(socket_fd, (void*)&c_len, sizeof(c_len), 0) < 0) {
        perror("receive");
        return 1;
    }

    c_len = ntohl(c_len);
    c = (uint32_t*) malloc(c_len * sizeof(uint32_t));

    if (recv(socket_fd, (void*)c, c_len * sizeof(uint32_t), 0) < 0) {
        perror("receive");
        return 1;
    }

    for (int i = 0; i < c_len; i++) {
        c[i] = ntohl(c[i]);
        printf("%d ", c[i]);
    }
    printf("\n");

    return 0;
}
