#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 10. The client sends to the server two strings. The server sends back the character
//     with the largest number of occurrences on the same positions in both strings together with its count.

int main() {
    int socket_fd, new_socket_fd, i, j;
    struct sockaddr_in server, client;
    uint32_t a_len, b_len, c_len, occurrences[26]={0};
    char *a, *b, *c;

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

    a = (char*) malloc(a_len * sizeof(char));
    if (recv(new_socket_fd, (void*)a, a_len * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    if (recv(new_socket_fd, (void*)&b_len, sizeof(b_len), 0) < 0) {
        perror("receive");
        return 1;
    }
    b_len = ntohl(b_len);

    b = (char*) malloc(b_len * sizeof(char));
    if (recv(new_socket_fd, (void*)b, b_len * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    for (i = 0; i < strlen(a); i++) {
        if (i >= strlen(b)) {
            break;
        }
        if (a[i] < 'a' || a[i] > 'z') {
            continue;
        }
        if (a[i] == b[i]) {
            occurrences[a[i] - 'a'] += 1;
        }
    }

    uint32_t max_app = occurrences[0];
    char max_app_char = 'a';
    for (i = 1; i < 26; i++) {
        if (occurrences[i] > max_app) {
            max_app = occurrences[i];
            max_app_char = 'a' + i;
        }
    }

    if (send(new_socket_fd, (void*)&max_app_char, sizeof(max_app_char), 0) < 0) {
        perror("send");
        return 1;
    }
    if (send(new_socket_fd, (void*)&max_app, sizeof(max_app), 0) < 0) {
        perror("send");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
