#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777
#define BACKLOG 5

// 6.   The client sends to the server a string and a character. The server returns to the
//      client a list of all positions in the string where specified character is found.

int main() {
    int socket_fd, new_socket_fd;
    struct sockaddr_in server, client;
    char c, *s;
    uint32_t *pos, s_len, cnt = 0;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&option, sizeof(option));

    server.sin_family = AF_INET;
    server.sin_port = htons(MYPORT);
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
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

    if (recv(new_socket_fd, (void*)&s_len, sizeof(s_len), 0) < 0) {
        perror("receive");
        return 1;
    }
    s_len = ntohl(s_len);

    s = (char*) malloc(s_len * sizeof(char));
    if (recv(new_socket_fd, (void*)s, s_len * sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    if (recv(new_socket_fd, (void*)&c, sizeof(char), 0) < 0) {
        perror("receive");
        return 1;
    }

    for (int i = 0; i < s_len; i++) {
        if (s[i] == c) {
            cnt += 1;
        }
    }

    pos = (uint32_t*) malloc(cnt * sizeof(uint32_t));
    int index = 0;
    for (int i = 0; i < s_len; i++) {
        if (s[i] == c) {
            pos[index] = i;
            pos[index] = htonl(pos[index]);
            index += 1;
        }
    }

    cnt = htonl(cnt);
    if (send(new_socket_fd, (void*)&cnt, sizeof(cnt), 0) < 0) {
        perror("send");
        return 1;
    }

    cnt = ntohl(cnt);
    if (send(new_socket_fd, (void*)pos, cnt * sizeof(uint32_t), 0) < 0) {
        perror("send");
        return 1;
    }

    close(new_socket_fd);
    close(socket_fd);

    return 0;
}
