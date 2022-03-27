#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MYPORT 7777

// 6.   The client sends to the server a string and a character. The server returns to the
//      client a list of all positions in the string where specified character is found.

int main() {
    int socket_fd;
    struct sockaddr_in server;
    char* s = "ana are mere";
    char c = 'a';
    uint32_t s_len = strlen(s), cnt, *pos;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("192.168.1.103");
    memset(&(server.sin_zero), 0, 8);
    server.sin_port = htons(MYPORT);
    server.sin_family = AF_INET;

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }

    s_len = htonl(s_len);
    if (send(socket_fd, (void*)&s_len, sizeof(s_len), 0) < 0) {
        perror("send");
        return 1;
    }
    s_len = ntohl(s_len);

    if (send(socket_fd, (void*)s, s_len * sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    if (send(socket_fd, (void*)&c, sizeof(char), 0) < 0) {
        perror("send");
        return 1;
    }

    if (recv(socket_fd, (void*)&cnt, sizeof(cnt), 0) < 0) {
        perror("receive");
        return 1;
    }
    cnt = ntohl(cnt);

    pos = (uint32_t*) malloc(cnt * sizeof(uint32_t));
    if (recv(socket_fd, (void*)pos, cnt * sizeof(uint32_t), 0) < 0) {
        perror("receive");
        return 1;
    }

    close(socket_fd);

    for (int i = 0; i < cnt; i++) {
        pos[i] = ntohl(pos[i]);
        printf("%d ", pos[i]);
    }
    printf("\n");

    return 0;
}
