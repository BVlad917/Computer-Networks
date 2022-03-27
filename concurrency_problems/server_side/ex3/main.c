#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/unistd.h>
#include <float.h>

#define PORT 9090
#define BACKLOG 10

// 3. The server chooses a random float number <SRF>. Run multiple clients. Each client chooses a
// random float number <CRF> and send it to the server. When the server does not receive any incoming
// connection for at least 10 seconds it chooses the client that has guessed the best approximation (is closest)
// for its own number and sends it back the message “You have the best guess with an error of <SRV>-<CRF>”.
// It also sends to each other client the string “You lost !”. The server closes all connections after this.

int main() {
    srand(time(NULL));  // set the seed
    float SRF = ((float)rand() / (float)RAND_MAX) * 10;     // generate a float number in the interval [0..10]
    printf("Generated SRF = %f\n", SRF);

    int socket_fd, new_socket_fd;
    struct sockaddr_in server, client;

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
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;

    if (bind(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind");
        return -3;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("failed to start listening for new connections");
        return -4;
    }

    fd_set master;
    FD_ZERO(&master);   // clear the master fd_set
    FD_SET(socket_fd, &master);     // add the rendezvous socket to the master fd_set
    int fd_max = socket_fd;     // at all times keep track of the maximum file descriptor; so far we have just the rendezvous socket
    fd_set read_fds;     // the fd_set will be modified after every <select> call so we need to copy the master every time
    struct timeval tv;   // the 10 seconds time interval. NB: after select the time interval should be considered NULL! So we need to reset it
    int i, select_return, client_size;

    float num;
    uint32_t tmp;

    float closest_number = FLT_MAX;
    int fd_of_closest_number = -1;

    char buff[256];
    memset(buff, 0, sizeof(buff));

    while (1) {
        // copy the master fd_set into the read fd_set
        read_fds = master;
        // reinitialize the time interval
        memset(&tv, 0, sizeof(tv));
        tv.tv_sec = 10;

        printf("Listening...\n");
        select_return = select(fd_max + 1, &read_fds, NULL, NULL, &tv);
        if (select_return < 0) {
            perror("failed to select file descriptor sets");
            continue;
        }
        else if (select_return == 0) {
            printf("No client connected for 10 seconds. The server will now choose the winner.\n");
            for (i = 0; i <= fd_max; i++) {
                if (FD_ISSET(i, &master) && i != socket_fd) {     // if this fd is one which we listened to AND it's not the rendezvous socket
                    if (i == fd_of_closest_number) {    // if winner fd, send "winner" message
                        sprintf(buff, "You have the best guess with an error of %f", fabs(SRF - closest_number));
                        if (send(i, (void*)buff, sizeof(buff), 0) < 0) {
                            perror("failed to send winning message");
                        }
                    }
                    else {      // if loser fd, send "lost" message
                        sprintf(buff, "You lost!");
                        if (send(i, (void*)buff, sizeof(buff), 0) < 0) {
                            perror("failed to send winning message");
                        }
                    }
                    close(i);
                    FD_CLR(i, &master);
                }
            }
            close(socket_fd);
            return 0;
        }

        for (i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fds)) {   // something from the read fd_set was triggered!
                if (i == socket_fd) {   // we got a new client trying to connect
                    client_size = sizeof(client);
                    new_socket_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
                    if (new_socket_fd < 0) {
                        perror("failed to accept new client connection");
                    }
                    else {
                        printf("Client %s connected on socket %d\n", inet_ntoa(client.sin_addr), new_socket_fd);
                        // now we need to add this client's fd to the read set s.t., we listen to incoming connections from this new client
                        FD_SET(new_socket_fd, &master);
                        if (new_socket_fd > fd_max) {   // maybe we need to update the maximum fd
                            fd_max = new_socket_fd;
                        }
                    }
                }
                else {      // incoming data from a client
                    if (recv(i, (void*)&tmp, sizeof(tmp), 0) < 0) {
                        perror("failed to receive float from client");
                        continue;
                    }
                    tmp = ntohl(tmp);
                    memcpy(&num, &tmp, sizeof(num));

                    if (fabs(num - SRF) < fabs(closest_number - SRF)) {
                        closest_number = num;
                        fd_of_closest_number = i;
                    }
                }
            }
        }
    }
}
