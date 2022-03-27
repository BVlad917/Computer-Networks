#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5555

// 2. The client sends the complete path to a file. The server returns back the length of the file and its
//    content in the case the file exists. When the file does not exist the server returns a length of -1
//    and no content. The client will store the content in a file with the same name as the input file with
//    the suffix –copy appended (ex: for f.txt => f.txt-copy).

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Need a file path\n");
        return -1;
    }
    char filepath[256];
    strcpy(filepath, argv[1]);

    int socket_fd;
    struct sockaddr_in server;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return -2;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        return -3;
    }

    printf("Connected to server\n");

    if (send(socket_fd, (void*)filepath, sizeof(filepath), 0) < 0) {
        perror("failed to send filepath");
        return -4;
    }

    char file_content[8192];
    memset(file_content, 0, sizeof(file_content));
    uint32_t length;
    int length_as_int;

    if (recv(socket_fd, (void*)&length, sizeof(length), 0) < 0) {
        perror("failed to receive the file length");
        return -5;
    }
    length = ntohl(length);
    length_as_int = (int)(length);

    if (recv(socket_fd, (void*)file_content, sizeof(file_content), 0) < 0) {
        perror("failed to receive the file content");
        return -6;
    }

    if (length_as_int == -1) {
        printf("The file does not exist.\n");
    }
    else {
        strcat(filepath, "-copy");
        FILE* fd = fopen(filepath, "w");
        fputs(file_content, fd);
        fclose(fd);
    }

    close(socket_fd);

    return 0;
}