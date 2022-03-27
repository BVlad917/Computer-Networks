#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define PORT 9090

// 3. The server chooses a random float number <SRF>. Run multiple clients. Each client chooses a
// random float number <CRF> and send it to the server. When the server does not receive any incoming
// connection for at least 10 seconds it chooses the client that has guessed the best approximation (is closest)
// for its own number and sends it back the message “You have the best guess with an error of <SRV>-<CRF>”.
// It also sends to each other client the string “You lost !”. The server closes all connections after this.


int main() {
    int socket_fd;
    struct sockaddr_in server;
    uint32_t tmp;

    srand(time(NULL));
    float crf = ((float)rand() / (float)RAND_MAX) * 10;
    printf("Generated CRF = %f\n", crf);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("192.168.1.102");

    printf("Connecting to server...\n");
    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("failed to connect to server");
        return -2;
    }
    printf("Connected\n");


    memcpy(&tmp, &crf, sizeof(tmp));
    uint32_t data = htonl(tmp);

    if (send(socket_fd, (void*) &data, sizeof(data), 0) < 0) {
        perror("failed to send crf to server");
        return -3;
    }

    char buff[256];
    memset(buff, 0, sizeof(buff));

    if (recv(socket_fd, (void*)&buff, sizeof(buff), 0) < 0) {
        perror("failed to receive server message");
        return -4;
    }

    close(socket_fd);

    printf("%s\n", buff);

    return 0;
}
