#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>

#define PORT 8888

// 6. The server chooses a random integer number. Each client generates a random integer number and
//    send it to the server. The server answers with the message “larger” if the client has sent a smaller
//    number than the server’s choice, or with message “smaller” if the client has sent a larger number than
//    the server’s choice. Each client continues generating a different random number (larger or smaller than
//    the previous) according to the server’s indication. When a client guesses the server choice – the server
//    sends back to the winner the message “You win – within x tries”. It also sends back to all other clients
//    the message “You lost – after y retries!” (x and y are the number of tries of each respective client).
//    The server closes all connections upon a win, and it chooses a different random integer for the
//    next game (set of clients)

int main() {
    int socket_fd, random_nr, guessed = 0;
    struct sockaddr_in server;
    uint32_t random_nr_as_uint;
    char msg[256];

    int lower_bound = 0;
    int upper_bound = 1000000;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.102");

    printf("Connecting to server...\n");
    if (connect(socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("connect");
        return -2;
    }
    printf("Connected.\n");

    srand(time(NULL));
    while (guessed == 0) {
        random_nr = rand() % (upper_bound - lower_bound + 1) + lower_bound; // generate numbers in the range [lower_bound, upper_bound]
        memcpy(&random_nr_as_uint, &random_nr, sizeof(uint32_t));
        random_nr_as_uint = htonl(random_nr_as_uint);

        printf("Generated %d\n", random_nr);

        if (send(socket_fd, (void*)&random_nr_as_uint, sizeof(random_nr_as_uint), 0) < 0) {
            perror("failed to send random number to server");
            continue;
        }

        if (recv(socket_fd, (void*)msg, sizeof(msg), 0) < 0) {
            perror("failed to receive message from the server");
            continue;
        }

        if (strcmp(msg, "smaller") == 0) {
            upper_bound = random_nr - 1;
        }
        else if (strcmp(msg, "larger") == 0) {
            lower_bound = random_nr + 1;
        }
        else {
            // game ended
            printf("%s\n", msg);
            guessed = 1;
        }
        usleep(250000);     // sleep 0.25 seconds
    }

    return 0;
}
