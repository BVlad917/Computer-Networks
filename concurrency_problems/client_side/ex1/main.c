#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5353

//1. The client takes a string from the command line and sends it to the server. The server interprets
//   the string as a command with its parameters. It executes the command and returns the standard
//   output and the exit code to the client.

int main(int argc, char* argv[]) {
    int socket_fd, i, int_exit_code;
    struct sockaddr_in server;
    char cmd[1024], out[1024];
    uint32_t exit_code;

    if (argc < 2) {
        printf("Command needed\n");
        return -1;
    }
    // put the command in cmd - char array
    memset(cmd, 0, sizeof(cmd));
    for (i = 1; i < argc; i++) {
        strcat(cmd, argv[i]);
        if (i == argc - 1) {
            strcat(cmd, "\0");
        } else {
            strcat(cmd, " ");
        }
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return -2;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("192.168.1.100");

    if (connect(socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("connect");
        return -3;
    }

    printf("Connected\n");

    if (send(socket_fd, (void*) &cmd, sizeof(cmd), 0) < 0) {
        perror("send");
        return -4;
    }

    if (recv(socket_fd, (void*)out, sizeof(out), 0) < 0) {
        perror("receive output");
        return -5;
    }

    if (recv(socket_fd, (void*)&exit_code, sizeof(exit_code), 0) < 0) {
        perror("receive exit code");
        return -6;
    }
    exit_code = ntohl(exit_code);
    int_exit_code = (int)(exit_code);

    printf("standard output:\n%s\n", out);
    printf("exit code:\n%d\n", int_exit_code);

    printf("Closing connection...\n");
    close(socket_fd);

    return 0;
}
