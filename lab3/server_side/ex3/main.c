#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/unistd.h>
#include <stdlib.h>

#define SRV_PORT 3535
#define BACKLOG 10

// 3. Implement the Chat server example (see the link bellow) using UDP and TCP – only this time each
//    client should contact the server just for registration. All communication happens directly between the peers
//    (clients) without passing through the server. Each client maintains an endpoint (TCP/UDP) with
//    the server just for learning the arrival/departure of other clients. You create a mesh architecture where
//    all clients connect directly between each others.

int main() {
    int socket_fd, new_socket_fd;
    struct sockaddr_in server, client, clients_udp[BACKLOG];
    uint32_t num_clients = 0;
    socklen_t client_size;
    char msg[256];

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    int yes_reuse_server = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes_reuse_server, sizeof(yes_reuse_server)) < 0) {
        perror("failed to set reuse server socket option");
        return -2;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SRV_PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("bind");
        return -3;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        return -4;
    }

    fd_set master, read_fd;
    FD_ZERO(&master);
    FD_SET(socket_fd, &master);
    FD_SET(0, &master);
    int i, j, fd_max = socket_fd;

    struct sockaddr_in disconnect, newly_connected_udp, newly_disconnected_udp;
    socklen_t disconnect_size;
    char disconnect_msg[256], connect_msg[256], console_msg[256];
    _ssize_t bytes_read, return_code;

    while (1) {
        printf("Listening...\n");

        read_fd = master;
        if (select(fd_max + 1, &read_fd, NULL, NULL, NULL) < 0) {
            perror("failed to select");
            continue;
        }

        for (i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fd)) {
                if (i == 0) {   // the server is trying to stop everything
                    bytes_read = read(0, console_msg, sizeof(console_msg) - 1);
                    console_msg[bytes_read - 1] = '\0';
                    if (strcmp(console_msg, "QUIT") == 0) {
                        printf("Closing the client connections...\n");
                        strcpy(disconnect_msg, "The server was shut down. You will be disconnected.");
                        // send a disconnect message to all connected clients
                        for (j = 1; j <= fd_max; j++) {     // stdin is also in the master set => start loop from 1, not from 0
                            if (FD_ISSET(j, &master) && j != socket_fd) {   // use <master> and not <read_fd> because the select function modified <read_fd>
                                return_code = send(j, (void*)disconnect_msg, sizeof(disconnect_msg), 0);
                                if (return_code < 0) {
                                    perror("failed to send disconnect message to the client");
                                }
                            }
                        }
                        // close all client sockets
                        for (j = 1; j <= fd_max; j++) {
                            if (FD_ISSET(j, &master) && j != socket_fd) {   // use <master> and not <read_fd> because the select function modified <read_fd>
                                close(j);
                            }
                        }
                        // close the rendezvous socket
                        close(socket_fd);
                        // printf message to server console
                        printf("Shutting down the server...\n");
                        printf("Goodbye!\n");
                        // stop everything
                        exit(1);
                    }
                }
                else if (i == socket_fd) {   // new client trying to connect
                    client_size = sizeof(client);
                    new_socket_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
                    if (new_socket_fd < 0) {
                        perror("failed to accept new connection");
                    }
                    else {
                        if (recv(new_socket_fd, (void*)&newly_connected_udp, sizeof(newly_connected_udp), 0) < 0) {
                            perror("failed to receive new client udp information");
                            continue;
                        }
                        clients_udp[num_clients++] = newly_connected_udp;

                        num_clients = htonl(num_clients);
                        if (send(new_socket_fd, (void*)&num_clients, sizeof(num_clients), 0) < 0) {
                            perror("failed to send number of clients to new client");
                            num_clients = ntohl(num_clients);
                            continue;
                        }
                        num_clients = ntohl(num_clients);

                        if (send(new_socket_fd, (void*)clients_udp, num_clients * sizeof(clients_udp[0]), 0) < 0) {
                            perror("failed to send clients list to new client");
                            continue;
                        }

                        // send a welcome message to the newly connected client
                        sprintf(connect_msg, "Welcome, user %s:%d! You were connected on the server socket %d.", inet_ntoa(client.sin_addr), client.sin_port, new_socket_fd);
                        if (send(new_socket_fd, (void*)connect_msg, sizeof(connect_msg), 0) < 0) {
                            perror("failed to send welcoming message to the newly connected client");
                        }

                        FD_SET(new_socket_fd, &master);
                        if (new_socket_fd > fd_max) {
                            fd_max = new_socket_fd;
                        }

                        sprintf(connect_msg, "Client %s:%d connected on socket %d", inet_ntoa(client.sin_addr), client.sin_port, new_socket_fd);
                        printf("%s\n", connect_msg);
                        // notify the other clients that a new client connected
                        for (j = 1; j <= fd_max; j++) {     // start from 1 so we ignore the server standard input
                            if (FD_ISSET(j, &master) && j != socket_fd && j != new_socket_fd) {     // use <master> and not <read_fd> because the select function modified <read_fd>
                                if (send(j, (void*)connect_msg, sizeof(connect_msg), 0) < 0) {
                                    perror("failed to send notifying message to the other clients message");
                                }
                            }
                        }

                        for (j = 1; j <= fd_max; j++) {
                            if (FD_ISSET(j, &master) && j != socket_fd && j != new_socket_fd) {
                                num_clients = htonl(num_clients);
                                if (send(j, (void*)&num_clients, sizeof(num_clients), 0) < 0) {
                                    perror("failed to send number of clients to client");
                                    continue;
                                }
                                num_clients = ntohl(num_clients);
                                if (send(j, (void*)clients_udp, num_clients * sizeof(clients_udp[0]), 0) < 0) {
                                    perror("failed to send list of clients to clients");
                                    continue;
                                }
                            }
                        }
                    }
                }
                else {  // this has to be an exit request from a client
                    if (recv(i, (void*)msg, sizeof(msg), 0) < 0) {
                        perror("failed to receive message from client");
                        continue;
                    }
                    if (strcmp(msg, "QUIT") == 0) {
                        disconnect_size = sizeof(disconnect);
                        if (getpeername(i, (struct sockaddr*) &disconnect, &disconnect_size) < 0) {
                            perror("failed to get disconnecting peer information");
                            continue;
                        }
                        sprintf(disconnect_msg, "Client %s:%d disconnected", inet_ntoa(disconnect.sin_addr), disconnect.sin_port);
                        printf("%s\n", disconnect_msg);

                        if (recv(i, (void*)&newly_disconnected_udp, sizeof(newly_disconnected_udp), 0) < 0) {
                            perror("failed to receive the newly disconnected udp client");
                            continue;
                        }
                        for (j = 0; j < num_clients; j++) {
                            if (clients_udp[i].sin_port == newly_disconnected_udp.sin_port && clients_udp[i].sin_addr.s_addr == newly_disconnected_udp.sin_addr.s_addr) {
                                for (int k = j; k < num_clients - 1; k++) {
                                    clients_udp[k] = clients_udp[k + 1];
                                }
                                break;
                            }
                        }
                        num_clients = num_clients - 1;

                        for (j = 1; j <= fd_max; j++) {     // start from 1, so we ignore server stdin
                            if (FD_ISSET(j, &master) && j != socket_fd && j != i) {     // use <master> and not <read_fd> because the select function modified <read_fd>
                                if (send(j, (void*)disconnect_msg, sizeof(disconnect_msg), 0) < 0) {
                                    perror("failed to send notifying message to the other clients message");
                                }
                            }
                        }

                        for (j = 1; j <= fd_max; j++) {
                            if (FD_ISSET(j, &master) && j != socket_fd && j != i) {
                                num_clients = htonl(num_clients);
                                if (send(j, (void*)&num_clients, sizeof(num_clients), 0) < 0) {
                                    perror("failed to send number of clients to client");
                                    continue;
                                }
                                num_clients = ntohl(num_clients);
                                if (send(j, (void*)clients_udp, num_clients * sizeof(clients_udp[0]), 0) < 0) {
                                    perror("failed to send list of clients to clients");
                                    continue;
                                }
                            }
                        }

                        close(i);
                        FD_CLR(i, &master);
                    }
                }
            }
        }
    }
}