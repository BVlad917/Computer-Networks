#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777

// 10. The client sends to the server two strings. The server sends back the character
//     with the largest number of occurrences on the same positions in both strings together with its count.

int main() {
    int socket_fd;
    struct sockaddr_in server;
    char *a = "adcnhjalotqandmd";
    char *b = "pdcnjtaptnmandmd";
    char c;
    uint32_t a_len = strlen(a), b_len = strlen(b), c_occurrences;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
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
    if (send(socket_fd, (void*)a, a_len * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    b_len = htonl(b_len);
    if (send(socket_fd, (void*)&b_len, sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    b_len = ntohl(b_len);
    if (send(socket_fd, (void*)b, b_len * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    if (recv(socket_fd, (void*)&c, sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }
    if (recv(socket_fd, (void*)&c_occurrences, sizeof(c_occurrences), 0) < 0) {
        perror("receive");
        return 1;
    }

    close(socket_fd);
    printf("%c\n", c);
    printf("%d\n", c_occurrences);

    return 0;
}
