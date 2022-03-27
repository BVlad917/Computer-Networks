#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT 5353
#define BACKLOG 5

// 1. The client takes a string from the command line and sends it to the server. The server interprets
//    the string as a command with its parameters. It executes the command and returns the standard
//    output and the exit code to the client.

int socket_fd;

void interrupt_signal(int sig) {
    close(socket_fd);
    exit(0);
}

void treat_client(int new_socket_fd) {
    char cmd[1024];
    char line[64];
    if (recv(new_socket_fd, (void*)cmd, sizeof(cmd), 0) < 0) {
        perror("failed to receive command");
        exit(-4);
    }

    FILE* fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("failed to run command\n");
        send(new_socket_fd, "failed to run command\0", 22, 0);
        exit(-5);
    }

    memset(cmd, 0, sizeof(cmd));
    while (fgets(line, sizeof(line), fp) != NULL) {
        strcat(cmd, line);
    }
    cmd[strlen(cmd)] = '\0';

    int status = pclose(fp);
    uint32_t exit_code = -1;
    if (WIFEXITED(status)) {    // if the process exited in normal conditions (not with CTRL+C etc...)
        exit_code = WEXITSTATUS(status);
    }

    if (send(new_socket_fd, (void*)cmd, sizeof(cmd), 0) < 0) {
        perror("failed to send standard output");
        exit(-6);
    }

    if (send(new_socket_fd, (void*)&exit_code, sizeof(exit_code), 0) < 0) {
        perror("failed to send exit code");
        exit(-7);
    }
}

int main() {
    int new_socket_fd;
    struct sockaddr_in server, client;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }
    const int yes = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &yes, sizeof(yes));

    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr*) &server, sizeof(server)) == -1) {
        perror("bind");
        return -2;
    }

    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        return -3;
    }

    int socket_size;
    signal(SIGCHLD, SIG_IGN);   // with this we won't need to wait for child processes
    signal(SIGINT, interrupt_signal);   // with this the rendezvous socket will be closed when forcefully closing the server

    while (1) {
        socket_size = sizeof(client);
        printf("Listening...\n");
        new_socket_fd = accept(socket_fd, (struct sockaddr*) &client, (socklen_t*) &socket_size);
        if (new_socket_fd == -1) {
            perror("failed to accept new connection");
            continue;
        }

        printf("Client %s connected\n", inet_ntoa(client.sin_addr));

        if (fork() == 0) {
            // child process
            close(socket_fd);   // no need for rendezvous socket in the child process
            treat_client(new_socket_fd);
            close(new_socket_fd);
            exit(0);
        }
        // parent process
        close(new_socket_fd);    // parent process doesn't need the new socket file descriptor
    }
}
