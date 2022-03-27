#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>

#define PORT 2222
#define BACKLOG 5

// 5. The client sends a domain name taken from the command line (Ex: www.google.com) to the server. The
// server opens a TCP connection to the IP address corresponding to the received domain name on
// port 80 (called HTTP-Srv). It sends on the TCP connection the string: “GET / HTTP/1.0\n\n” and relays the answer
// back to the client. When HTTP-Srv closes connection to the server, the server closes the connection to
// the client at its turn.

// todo: this is useful; can use it to get IPv4 address as char*
//    struct hostent* google_host = gethostbyname("www.google.com");
//    struct in_addr* address = (struct in_addr*)google_host->h_addr_list[0];
//    printf("%s\n", inet_ntoa(*address));

int socket_fd;

void treat_client(int new_socket_fd) {
    struct sockaddr_in http_srv;
    int http_srv_socket_fd;
    char domain[256], cmd[512], response[4096];

    if (recv(new_socket_fd, (void*)domain, sizeof(domain), 0) < 0) {
        perror("failed to receive command from client");
        exit(-5);
    }

    struct hostent* http_srv_info = gethostbyname(domain);
    if (http_srv_info == NULL) {
        printf("failed to get http server info");
        exit(-6);
    }

    http_srv_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (http_srv_socket_fd < 0) {
        perror("failed to create http server socket");
        exit(-7);
    }

    memset(&http_srv, 0, sizeof(http_srv));
    http_srv.sin_port = htons(80);
    http_srv.sin_family = AF_INET;
    memcpy(&http_srv.sin_addr, http_srv_info->h_addr, http_srv_info->h_length);

    memset(cmd, 0, sizeof(cmd));
    strcpy(cmd, "GET / HTTP/1.0\n\n");
    memset(response, 0, sizeof(response));

    if (connect(http_srv_socket_fd, (struct sockaddr*)&http_srv, sizeof(http_srv)) < 0) {
        perror("failed to connect to http server");
        exit(-8);
    }

    if (send(http_srv_socket_fd, (void*)cmd, sizeof(cmd), 0) < 0) {
        perror("failed to send the request to the http server");
        exit(-9);
    }

    if (recv(http_srv_socket_fd, (void*)response, sizeof(response), 0) < 0) {
        perror("failed to receive the response from the http server");
        exit(-10);
    }
    // after sending the data, the HTTP 1.0 server will close the connection, so there's no need for the socket anymore
    close(http_srv_socket_fd);

    // send the response to the client
    if (send(new_socket_fd, (void*)response, sizeof(response), 0) < 0) {
        perror("failed to send the response to the client");
        exit(-11);
    }
    exit(0);    // successful
}

void interrupt_signal(int sig) {
    close(socket_fd);
    exit(0);
}

int main() {
    int new_socket_fd, client_size;
    struct sockaddr_in server, client, http_srv;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    int yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0) {
        perror("failed to set socket option");
        return -2;
    }

    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("failed to bind");
        return -3;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("failed to listen");
        return -4;
    }

    signal(SIGCHLD, SIG_IGN);   // finished child processes will be ignored => we won't have to wait for them
    signal(SIGINT, interrupt_signal);   // when closing the server with CTRL+C we want the rendezvous socket to first close, then stop the process

    while (1) {
        printf("Listening...\n");

        client_size = sizeof(client);
        new_socket_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
        if (new_socket_fd < 0) {
            perror("failed to accept new client connection");
            continue;
        }
        printf("Client %s connected.\n", inet_ntoa(client.sin_addr));

        if (fork() == 0) {  // if in child process
            close(socket_fd);   // close rendezvous socket
            treat_client(new_socket_fd);
            close(new_socket_fd);   // close child process socket so the connection to the client is closed
            exit(0);
        }
        close(new_socket_fd);    // if in parent process, close the new socket file descriptor
    }
}