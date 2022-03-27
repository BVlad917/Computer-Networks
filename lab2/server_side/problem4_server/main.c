#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 4.   The client send to the server two sorted array of chars. The server will merge sort the two
//      arrays and return the result to the client.

int main() {
    int socket_fd, new_socket_fd;
    struct sockaddr_in server, client;
    uint32_t len_s1, len_s2, len_s3;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }
    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(option));

    server.sin_family = AF_INET;
    server.sin_port = htons(MYPORT);
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
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

    if (recv(new_socket_fd, (void *)&len_s1, sizeof(len_s1), 0) < 0) {
        perror("receive");
        return 1;
    }
    len_s1 = ntohl(len_s1);

    char* s1 = (char*) malloc(len_s1 * sizeof(char));
    if (recv(new_socket_fd, (void*)s1, len_s1 * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    if (recv(new_socket_fd, (void *)&len_s2, sizeof(len_s2), 0) < 0) {
        perror("receive");
        return 1;
    }
    len_s2 = ntohl(len_s2);

    char* s2 = (char*) malloc(len_s2 * sizeof(char));
    if (recv(new_socket_fd, (void*)s2, len_s2 * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    // merge the 2 arrays
    len_s3 = len_s1 + len_s2;
    char* s3 = (char*) malloc(len_s3 * sizeof(char));
    int i = 0, j = 0, k = 0;
    while (i < len_s1 && j < len_s2) {
        if (s1[i] < s2[j]) {
            s3[k] = s1[i];
            i += 1;
        } else {
            s3[k] = s2[j];
            j += 1;
        }
        k += 1;
    }
    while (i < len_s1) {
        s3[k] = s1[i];
        k += 1;
        i += 1;
    }
    while (j < len_s2) {
        s3[k] = s2[j];
        k += 1;
        j += 1;
    }

    len_s3 = htonl(len_s3);
    if (send(new_socket_fd, (void*)&len_s3, sizeof(len_s3), 0) < 0) {
        perror("send");
        return 1;
    }
    len_s3 = ntohl(len_s3);
    if (send(new_socket_fd, (void*)s3, len_s3 * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
