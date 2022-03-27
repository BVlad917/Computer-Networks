#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/unistd.h>

#define PORT 8888
#define BACKLOG 10

// 6. The server chooses a random integer number. Each client generates a random integer number and
//    send it to the server. The server answers with the message “larger” if the client has sent a smaller
//    number than the server’s choice, or with message “smaller” if the client has sent a larger number than
//    the server’s choice. Each client continues generating a different random number (larger or smaller than
//    the previous) according to the server’s indication. When a client guesses the server choice – the server
//    sends back to the winner the message “You win – within x tries”. It also sends back to all other clients
//    the message “You lost – after y retries!” (x and y are the number of tries of each respective client).
//    The server closes all connections upon a win, and it chooses a different random integer for the
//    next game (set of clients)

pthread_mutex_t guess_num_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t restart_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[BACKLOG];
pthread_t restart_game_thread;
int lower_bound = 0;
int upper_bound = 1000000;
int num_threads, random_nr, guessed, num_clients;
int clients_fd[BACKLOG];

void* treat_client(void* arg) {
    int new_socket_fd = *((int*)arg);
    uint32_t num;
    int num_as_int, i_won = 0, num_tries = 0;
    char msg[256];

    while (guessed == 0) {
        if (recv(new_socket_fd, (void*)&num, sizeof(num), 0) < 0) {
            perror("failed to receive number from client");
            continue;
        }
        num = ntohl(num);
        memcpy(&num_as_int, &num, sizeof(int));
//        num_as_int = (int)num;
        num_tries = num_tries + 1;

        if (random_nr < num_as_int) {
            strcpy(msg, "smaller\0");
            if (send(new_socket_fd, (void*)msg, sizeof(msg), 0) < 0) {
                perror("failed to send message to client");
                continue;
            }
        }
        else if (random_nr > num_as_int) {
            strcpy(msg, "larger\0");
            if (send(new_socket_fd, (void*)msg, sizeof(msg), 0) < 0) {
                perror("failed to send message to client");
                continue;
            }
        }
        else {  // random_nr == num_as_int => this client guessed correctly
            if (guessed == 1) {
                break;      // what if 2 clients guess the number at EXACTLY the same time?
            }
            pthread_mutex_lock(&guess_num_mtx);
            guessed = 1;
            pthread_mutex_unlock(&guess_num_mtx);

            i_won = 1;
        }
    }

    if (i_won) {
        sprintf(msg, "You win – within %d tries", num_tries);
        pthread_mutex_unlock(&restart_mtx);     // notify the background thread that it can run
    }
    else {
        sprintf(msg, "You lost – after %d retries!", num_tries);
    }

    if (send(new_socket_fd, (void*)msg, sizeof(msg), 0) < 0) {
        perror("failed to send termination message to client");
        return NULL;
    }

    return NULL;
}

void* restart_game(void* arg) {
    int i;
    while (1) {
        pthread_mutex_lock(&restart_mtx);   // will block until someone wins
        for (i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);     // join all the client threads
        }

        for (i = 0; i < num_clients; i++) {
            close(clients_fd[i]);   // close all the clients sockets
        }

        // reinitialize the state of the game
        guessed = 0;
        num_threads = 0;
        num_clients = 0;
        random_nr = rand() % (upper_bound - lower_bound + 1) + lower_bound; // generate numbers in the range [lower_bound, upper_bound]
        printf("Server number = %d\n", random_nr);
    }
}

int main() {
    struct sockaddr_in server, client;
    int socket_fd, new_socket_fd;

    srand(time(NULL));
    random_nr = rand() % (upper_bound - lower_bound + 1) + lower_bound; // generate numbers in the range [lower_bound, upper_bound]
    printf("Server number = %d\n", random_nr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create the socket");
        return -1;
    }

    int yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0) {
        perror("failed to set socket option");
        return -2;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("failed to bind");
        return -3;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("failed to listen for new client connections");
        return -4;
    }

    pthread_mutex_lock(&restart_mtx);
    if (pthread_create(&restart_game_thread, NULL, restart_game, NULL) != 0) {
        printf("Failed to create restart game thread");
        return -5;
    }
    pthread_detach(restart_game_thread);    // detach the background thread

    int client_size;
    while (1) {
        printf("Listening...\n");

        client_size = sizeof(client);
        new_socket_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
        if (new_socket_fd < 0) {
            perror("failed to accept new connection");
            continue;
        }
        printf("Client %s connected.\n", inet_ntoa(client.sin_addr));
        clients_fd[num_clients++] = new_socket_fd;  // save the fd so that we can later close it in the background thread

        // also save the thread ID so that we can later join it in the background thread
        if (pthread_create(&threads[num_threads++], NULL, treat_client, (void*)&new_socket_fd) != 0) {
            printf("Failed to create thread\n");
            continue;
        }

    }
}
