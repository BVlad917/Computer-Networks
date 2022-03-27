#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 5. The client sends to the server an integer. The server returns the list of divisors for the specified number.

int main() {
    int socket_fd, new_socket_fd;
    struct sockaddr_in server, client;
    uint32_t n, nr_divs = 2;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &option, sizeof(option));

    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(MYPORT);
    server.sin_family = AF_INET;
    memset(&(server.sin_zero), 0, 8);

    if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    int socket_size = sizeof(client);
    new_socket_fd = accept(socket_fd, (struct sockaddr *) &client, &socket_size);
    if (new_socket_fd < 0) {
        perror("accept");
        return 1;
    }

    if (recv(new_socket_fd, (void *) &n, sizeof(n), 0) < 0) {
        perror("receive");
        return 1;
    }

    n = ntohl(n);

    for (int div = 2; div <= n / 2; div++) {
        if (n % div == 0) {
            nr_divs += 1;
        }
    }

    uint32_t* divs = (uint32_t*) malloc(nr_divs * sizeof(uint32_t));
    divs[0] = 1;
    divs[nr_divs - 1] = n;

    int index = 1;
    for (uint32_t div = 2; div <= n / 2; div++) {
        if (n % div == 0) {
            divs[index] = div;
            index += 1;
        }
    }

    for (int i = 0; i < nr_divs; i++) {
        divs[i] = htonl(divs[i]);
    }

    nr_divs = htonl(nr_divs);
    if (send(new_socket_fd, (void*)&nr_divs, sizeof(nr_divs), 0) < 0) {
        perror("send");
        return 1;
    }

    nr_divs = ntohl(nr_divs);
    if (send(new_socket_fd, (void*)divs, nr_divs * sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
