#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#define PORT 3535

// 3. Implement the Chat server example (see the link bellow) using UDP and TCP – only this time each
//    client should contact the server just for registration. All communication happens directly between the peers
//    (clients) without passing through the server. Each client maintains an endpoint (TCP/UDP) with
//    the server just for learning the arrival/departure of other clients. You create a mesh architecture where
//    all clients connect directly between each others.

int socket_fd;

void handle_interrupt(int sig) {
    char my_msg[256];
    strcpy(my_msg, "QUIT\0");
    if (send(socket_fd, (void*)my_msg, sizeof(my_msg), 0) < 0) {
        perror("failed to send a message to the server");
    }
    close(socket_fd);
    exit(1);
}

int main() {
    ssize_t read_bytes;
    char connection_msg[256], my_msg[256], peer_msg[256];
    ssize_t my_msg_len;

    // set up the udp part
    int udp_socket_fd;
    struct sockaddr_in udp_socket, self_udp_address, sender;
    socklen_t self_udp_address_size, sender_size;

    udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd < 0) {
        perror("failed to create udp socket");
        return -1;
    }
    int yes_reuse = 1;
    if (setsockopt(udp_socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes_reuse, sizeof(yes_reuse)) < 0) {
        perror("failed to set reuse option in udp socket");
        return -2;
    }

    memset(&udp_socket, 0, sizeof(udp_socket));
    udp_socket.sin_family = AF_INET;
    udp_socket.sin_port = htons(8);
    udp_socket.sin_addr.s_addr = inet_addr("8.8.8.8");
    if (sendto(udp_socket_fd, "hey\0", 4, 0, (struct sockaddr*)&udp_socket, sizeof(udp_socket)) < 0) {
        perror("failed to send dummy message");
        return -3;
    }

    self_udp_address_size = sizeof(self_udp_address);
    if (getsockname(udp_socket_fd, (struct sockaddr*)&self_udp_address, &self_udp_address_size) < 0) {
        perror("failed to get local information");
        return -4;
    }
    // udp part done

    int i;
    struct sockaddr_in server;
    struct sockaddr_in clients_sockets[10];
    uint32_t num_clients_sockets;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.102");

    signal(SIGINT, handle_interrupt);

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("failed to connect to server");
        return -2;
    }

    if (send(socket_fd, (void*)&self_udp_address, sizeof(self_udp_address), 0) < 0) {
        perror("failed to send my information to the server");
        return -3;
    }

    if (recv(socket_fd, (void*)&num_clients_sockets, sizeof(num_clients_sockets), 0) < 0) {
        perror("failed to receive the number of clients after connecting");
        return -4;
    }
    num_clients_sockets = ntohl(num_clients_sockets);

    if (recv(socket_fd, (void*)clients_sockets, num_clients_sockets * sizeof(clients_sockets[0]), 0) < 0) {
        perror("failed to receive the clients list after connecting");
        return -5;
    }

    if (recv(socket_fd, (void*)connection_msg, sizeof(connection_msg), 0) < 0) {
        perror("failed to receive welcome message");
        return -6;
    }

    fd_set master, read_fd;
    FD_ZERO(&master);
    FD_SET(0, &master);
    FD_SET(socket_fd, &master);
    FD_SET(udp_socket_fd, &master);

    while (1) {
        read_fd = master;

        if (select(socket_fd + 1, &read_fd, NULL, NULL, NULL) < 0) {
            perror("failed to select");
            continue;
        }

        if (FD_ISSET(socket_fd, &read_fd)) {
            read_bytes = recv(socket_fd, (void*)connection_msg, sizeof(connection_msg), 0);
            if (read_bytes < 0) {
                perror("failed to receive connection message from server");
                continue;
            }
            else if (read_bytes == 0) {
                printf("The server closed the connection\n");
                close(socket_fd);
                exit(1);
            }
            else {
                printf("%s\n", connection_msg);
            }

            if (recv(socket_fd, (void*)&num_clients_sockets,sizeof(num_clients_sockets), 0) < 0) {
                perror("failed to receive number of clients from server");
                continue;
            }
            num_clients_sockets = ntohl(num_clients_sockets);

            if (recv(socket_fd, (void*)clients_sockets, sizeof(clients_sockets), 0) < 0) {
                perror("failed to receive list of clients from server");
                continue;
            }
        }

        if (FD_ISSET(0, &read_fd)) {
            my_msg_len = read(0, my_msg, sizeof(my_msg) - 1);
            my_msg[my_msg_len - 1] = '\0';
            if (strcmp(my_msg, "QUIT") == 0) {
                // If the client wrote "QUIT", that message has to be sent to the server
                if (send(socket_fd, (void*)my_msg, sizeof(my_msg), 0) < 0) {
                    perror("failed to send a message to the server");
                }

                if (send(socket_fd, (void*)&self_udp_address, sizeof(self_udp_address), 0) < 0) {
                    perror("failed to send my information to the server");
                    continue;
                }

            }
            else {
                // If the client wrote something other than "QUIT", that message has to be sent to the other clients
                for (i = 0; i < num_clients_sockets; i++) {
                    if (clients_sockets[i].sin_addr.s_addr == self_udp_address.sin_addr.s_addr && clients_sockets[i].sin_port == self_udp_address.sin_port) {
                        continue;
                    }
                    if (sendto(udp_socket_fd, (void *) my_msg, strlen(my_msg) + 1, 0, (struct sockaddr *) &clients_sockets[i], sizeof(clients_sockets[i])) < 0) {
                        perror("failed to send message to client");
                    }
                }
            }
        }

        if (FD_ISSET(udp_socket_fd, &read_fd)) {
            sender_size = sizeof(sender);
            if (recvfrom(udp_socket_fd, (void*)peer_msg, sizeof(peer_msg), 0, (struct sockaddr*)&sender, &sender_size) < 0) {
                perror("failed to get message from client");
                continue;
            }
            printf("%s\n", peer_msg);
        }
    }
}