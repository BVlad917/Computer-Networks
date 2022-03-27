#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777

// 5. The client sends to the server an integer. The server returns the list of divisors for the specified number.

int main() {
    int socket_fd;
    struct sockaddr_in server;
    uint32_t n = 32, nr_divs;

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

    n = htonl(n);
    if (send(socket_fd, (void*)&n, sizeof(n), 0) < 0) {
        perror("send");
        return 1;
    }

    if (recv(socket_fd, (void*)&nr_divs, sizeof(nr_divs), 0) < 0) {
        perror("receive");
        return 1;
    }
    nr_divs = ntohl(nr_divs);

    uint32_t* divs = (uint32_t*) malloc(nr_divs * sizeof(uint32_t));
    if (recv(socket_fd, (void*)divs, nr_divs * sizeof(uint32_t), 0) < 0) {
        perror("receive");
        return 1;
    }

    close(socket_fd);

    for (int i = 0; i < nr_divs; i++) {
        divs[i] = ntohl(divs[i]);
        printf("%d ", divs[i]);
    }
    printf("\n");

    return 0;
}
