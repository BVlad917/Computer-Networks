#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 2222

int main(int argc, char* argv[]) {
    int socket_fd;
    struct sockaddr_in server;
    char domain[256], response[4096];

    if (argc < 2) {
        printf("ERROR: Need a domain.\n");
        return -1;
    }
    strcpy(domain, argv[1]);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -2;
    }

    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.102");

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("failed to connect");
        return -3;
    }

    if (send(socket_fd, (void*)domain, sizeof(domain), 0) < 0) {
        perror("failed to send domain to server");
        return -4;
    }

    memset(response, 0, sizeof(response));
    if (recv(socket_fd, (void*)response, sizeof(response), 0) < 0) {
        perror("failed to receive the response from the server");
        return -5;
    }
    close(socket_fd);

    printf("%s\n", response);

    return 0;
}
