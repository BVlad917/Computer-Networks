#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>

#define PORT 5555
#define BACKLOG 5

// 2. The client sends the complete path to a file. The server returns back the length of the file and its
//    content in the case the file exists. When the file does not exist the server returns a length of -1
//    and no content. The client will store the content in a file with the same name as the input file with
//    the suffix –copy appended (ex: for f.txt => f.txt-copy).

int socket_fd, new_socket_fd;

void close_process(int sig) {
    close(new_socket_fd);
    close(socket_fd);
    exit(0);
}

void treat_client() {
    char filepath[256];
    if (recv(new_socket_fd, (void*)filepath, sizeof(filepath), 0) < 0) {
        perror("failed to receive file path");
        exit(-6);
    }

    char line[256], file_content[8192];
    memset(file_content, 0, sizeof(file_content));
    uint32_t length = -1;

    FILE* fp = fopen(filepath, "r");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        rewind(fp);

        while (fgets(line, sizeof(line), fp) != NULL) {
            strcat(file_content, line);
        }
        fclose(fp);
    }

    length = htonl(length);
    if (send(new_socket_fd, (void*)&length, sizeof(length), 0) < 0) {
        perror("failed to send the file length");
        exit(-7);
    }
    length = ntohl(length);

    if (send(new_socket_fd, (void*)file_content, sizeof(file_content), 0) < 0) {
        perror("failed to send the file content");
        exit(-8);
    }
}

int main() {
    struct sockaddr_in server, client;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return -1;
    }

    int yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0) {
        perror("set socket option");
        return -2;
    }

    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind");
        return -3;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return -4;
    }

    signal(SIGCHLD, SIG_IGN);   // ignore finished child processes so that now we don't need to wait for them
    signal(SIGINT, close_process);  // when forcibly closing the server we want the sockets to close as well

    while (1) {
        printf("Listening...\n");
        socklen_t client_size = sizeof(client);
        new_socket_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
        if (new_socket_fd < 0) {
            perror("failed to accept new client connection");
            continue;
        }
        printf("Client %s connected\n", inet_ntoa(client.sin_addr));

        if (fork() == 0) {
            close(socket_fd);   // child process doesn't need rendezvous socket
            treat_client();
            close(new_socket_fd);
            exit(0);
        }
        close(new_socket_fd);   // parent process doesn't need new socket file descriptor
    }
}
