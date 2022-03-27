#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define MYPORT 8080

// 1.   A client sends to the server an array of numbers. Server returns the sum of the numbers.

int main() {
    int socket_fd;
    struct sockaddr_in server_address;
    u_int32_t a[] = {1, 2, 3, 4, 5, 6, 17}, alen = 7, sum, host_repr_len;
    int bytes_sent, bytes_read;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(MYPORT);
    server_address.sin_addr.s_addr = inet_addr("192.168.1.102");
    memset(&(server_address.sin_zero), 0, 8);

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(struct sockaddr)) < 0) {
        perror("connect");
        return 1;
    }

    for (int i = 0; i < alen; i++) {
        a[i] = htonl(a[i]);
    }
    host_repr_len = alen;
    alen = htonl(alen);

    bytes_sent = send(socket_fd, (void *)&alen, sizeof(alen), 0);
    if (bytes_sent < 0) {
        perror("send (array length)");
        return 1;
    }

    bytes_sent = send(socket_fd, (void *)a, host_repr_len * sizeof(a[0]), 0);
    if (bytes_sent < 0) {
        perror("send (array)");
        return 1;
    }

    bytes_read = recv(socket_fd, (void *)&sum, sizeof(sum), 0);
    if (bytes_read < 0) {
        perror("receive");
        return 1;
    }

    sum = ntohl(sum);
    printf("The sum is: %d\n", sum);

    close(socket_fd);

    return 0;
}
