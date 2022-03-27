#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <sys/unistd.h>
#include <stdlib.h>

#define PORT 9191
#define BACKLOG 10

// 4. The clients send an integer number N and an array of N float values. The server will merge sort
//    the numbers received from all clients until it gets an empty array of floats (N=0). The server returns
//    to each client the size of the merge-sorted array followed by the merge-sorted arrays of all
//    floats from all clients.

float final_array[2048];
uint32_t final_array_len;
uint32_t final_array_as_ints[2048];
int num_threads, num_clients;
int socket_fd;      // the rendezvous socket
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[10];
int clients_fd[10];

void merge(float a[], int low, int mid, int high) {
    int i, j, k;
    int n1 = mid - low + 1;
    int n2 = high - mid;

    float* l = (float*) malloc(n1 * sizeof(float));
    float* r = (float*) malloc(n2 * sizeof(float));

    for (i = 0; i < n1; i++) {
        l[i] = a[low + i];
    }
    for (j = 0; j < n2; j++) {
        r[j] = a[mid + 1 + j];
    }

    i = 0, j = 0, k = low;
    while (i < n1 && j < n2) {
        if (l[i] <= r[j]) {
            a[k] = l[i];
            i = i + 1;
        }
        else {
            a[k] = r[j];
            j = j + 1;
        }
        k = k + 1;
    }
    while (i < n1) {
        a[k] = l[i];
        k = k + 1;
        i = i + 1;
    }
    while (j < n2) {
        a[k] = r[j];
        k = k + 1;
        j = j + 1;
    }

    free(l);
    free(r);
}

void mergesort(float a[], int low, int high) {
    if (low >= high) {
        return;
    }
    int mid = (low + high) / 2;
    mergesort(a, low, mid);
    mergesort(a, mid + 1, high);
    merge(a, low, mid, high);
}

void treat_good_client(uint32_t N, int new_socket_fd) {
    float nums[64];
    uint32_t nums_as_ints[64];
    // receive the array of floats as array of ints
    if (recv(new_socket_fd, (void*)nums_as_ints, N * sizeof(uint32_t), 0) < 0) {
        perror("failed to receive array of floats");
        return;
    }

    // convert the array of ints to an array of floats (don't forget to switch to the host representation with ntohl())
    for (int i = 0; i < N; i++) {
        nums_as_ints[i] = ntohl(nums_as_ints[i]);
        memcpy(&nums[i], &nums_as_ints[i], sizeof(float));
    }

    // now we need to add these floats to the global array of floats
    pthread_mutex_lock(&mtx);
    for (int i = 0; i < N; i++) {
        final_array[final_array_len] = nums[i];
        final_array_len = final_array_len + 1;
    }
    // now we have to sort the array, since we added new stuff
    mergesort(final_array, 0, final_array_len - 1);
    // unlock the mutex so the other threads can work as well
    pthread_mutex_unlock(&mtx);
}

void treat_ending_client() {
    int i;

    // this thread will stop everything
    // first, we need to wait for all the other threads to finish; this way we know that the global array is complete
    for (i = 0; i < num_threads; i++) {
        if (pthread_equal(pthread_self(), threads[i])) {
            continue;   // we don't wait for the current thread (this one, the one that's ending everything)
        }
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Failed to join thread\n");
            return;
        }
    }

    // now we are certain we have the complete array in <final_array>
    // we can destroy the mutex; don't need it anymore
    pthread_mutex_destroy(&mtx);

    // now we send the final array to all the clients
    // first let's convert the values of the final array to network representation
    for (i = 0; i < final_array_len; i++) {
        memcpy(&final_array_as_ints[i], &final_array[i], sizeof(uint32_t));
        final_array_as_ints[i] = htonl(final_array_as_ints[i]);
    }

    // now we can send this converted array
    for (i = 0; i < num_clients; i++) {
        // first send the final array length
        final_array_len = htonl(final_array_len);
        if (send(clients_fd[i], (void*)&final_array_len, sizeof(final_array_len), 0) < 0) {
            perror("failed to send final array length to client");
            continue;
        }
        final_array_len = ntohl(final_array_len);

        // now send the array (the array of ints, the client will have to convert it back)
        if (send(clients_fd[i], (void*)final_array_as_ints, final_array_len * sizeof(uint32_t), 0) < 0) {
            perror("failed to send the final array to client");
            continue;
        }
    }

    // now this thread can also close the sockets, including the rendezvous socket (we don't expect connection from now on)
    for (i = 0; i < num_clients; i++) {
        close(clients_fd[i]);
    }
    close(socket_fd);
    // the detached thread can also close the process since there is nothing else to do and main is stuck
    // in an infinite while loop
    exit(0);
}

void* treat_client(void* new_socket_fd_void) {
    uint32_t N;
    int new_socket_fd = *((int*) new_socket_fd_void);

    if (recv(new_socket_fd, (void*)&N, sizeof(N), 0) < 0) {
        perror("failed to receive N");
        return NULL;
    }
    N = ntohl(N);

    if (N == 0) {
        pthread_detach(pthread_self()); // don't need the return value of this thread; we won't have where to join it; so we detach it
        treat_ending_client();
    } else {
        treat_good_client(N, new_socket_fd);
    }
    return NULL;
}

int main() {
    int new_socket_fd;
    struct sockaddr_in server, client;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("failed to bind");
        return -2;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("failed to start listening for connections");
        return -3;
    }

    int client_size;
    while (1) {
        printf("Listening...\n");

        client_size = sizeof(client);
        new_socket_fd = accept(socket_fd, (struct sockaddr*) &client, &client_size);
        if (new_socket_fd < 0) {
            perror("failed to establish connection with new client");
            return -4;
        }
        clients_fd[num_clients] = new_socket_fd;
        num_clients = num_clients + 1;
        printf("Client %s connected, ID = %d\n", inet_ntoa(client.sin_addr), num_clients);

        if (pthread_create(&threads[num_threads++], NULL, treat_client, (void*) &new_socket_fd) != 0) {
            printf("Failed to create new thread\n");
            continue;
        }
    }
}
