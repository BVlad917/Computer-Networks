#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 7272

int main() {
    int socket_fd;
    struct sockaddr_in server;
    char msg[256];
    socklen_t server_size;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("192.168.1.102");

    memset(msg, 0, sizeof(msg));
    strcpy(msg, "hey man, 'sup?");

    if (sendto(socket_fd, (void*)msg, strlen(msg) + 1, 0, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("failed to send message to server");
        return -2;
    }

    server_size = sizeof(server);
    if (recvfrom(socket_fd, (void*)msg, sizeof(msg), 0, (struct sockaddr*) &server, &server_size) < 0) {
        perror("failed to receive confirmation message from server");
        return -3;
    }

    printf("%s\n", msg);

    return 0;
}
