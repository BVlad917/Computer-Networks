#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define PORT 7272

int main() {
    int socket_fd, from_length;
    struct sockaddr_in server, from;
    char buff[256];

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind");
        return -2;
    }

    while (1) {
        printf("Listening...");

        from_length = sizeof(from);
        if (recvfrom(socket_fd, (void*)buff, sizeof(buff), 0, (struct sockaddr*)&from, &from_length) < 0) {
            perror("failed to receive message from client");
            continue;
        }
        printf("Received a message from %s\n", inet_ntoa(from.sin_addr));

        printf("%s\n", buff);

        strcpy(buff, "got your message, g\0");
        if (sendto(socket_fd, (void*)buff, strlen(buff) + 1, 0, (struct sockaddr*)&from, sizeof(from)) < 0) {
            perror("failed to send confirmation message from client");
            continue;
        }
    }
}

#pragma clang diagnostic pop