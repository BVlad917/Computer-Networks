#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 9191

// 4. The clients send an integer number N and an array of N float values. The server will merge sort
//    the numbers received from all clients until it gets an empty array of floats (N=0). The server returns
//    to each client the size of the merge-sorted array followed by the merge-sorted arrays of all
//    floats from all clients.

int main(int argc, char* argv[]) {
    int socket_fd, i;
    struct sockaddr_in server;
    uint32_t N, full_array_length, nums_as_ints[64], full_array_as_ints[2048];
    float nums[64], full_array[2048];

    if (argc < 2) {
        printf("Need input of type: N f[0] f[1] ... f[N]\n");
        return -1;
    }

    N = atoi(argv[1]);
    if (N != argc - 2) {
        printf("N and the number of elements in the array do not match\n");
        return -2;
    }

    for (i = 2; i < argc; i++) {
        nums[i - 2] = atof(argv[i]);
    }

    // now we have the float numbers from the user in <nums> and the count in N
    // let's convert these float numbers to network representation so that we can send it through the socket

    for (i = 0; i < N; i++) {
        memcpy(&nums_as_ints[i], &nums[i], sizeof(uint32_t));
        nums_as_ints[i] = htonl(nums_as_ints[i]);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -3;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("192.168.1.102");

    printf("Connecting to server...\n");
    if (connect(socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("failed to connect to server");
        return -4;
    }
    printf("Connected.\n");

    // send N
    N = htonl(N);
    if (send(socket_fd, (void*) &N, sizeof(N), 0) < 0) {
        perror("failed to send N");
        return -5;
    }
    N = ntohl(N);

    // only send the array of floats if we have an array (i.e., N is not zero)
    if (N != 0) {
        // send the array of floats
        if (send(socket_fd, (void*)nums_as_ints, N * sizeof(uint32_t), 0) < 0) {
            perror("failed to send the array of floats");
            return -6;
        }
    }


    // now we receive the full array of sorted floats
    // first receive the length
    if (recv(socket_fd, (void*)&full_array_length, sizeof(full_array_length), 0) < 0) {
        perror("failed to receive the full array length");
        return -7;
    }
    full_array_length = ntohl(full_array_length);

    // now receive the array itself
    if (recv(socket_fd, (void*)full_array_as_ints, full_array_length * sizeof(uint32_t), 0) < 0) {
        perror("failed to receive the full array");
        return -8;
    }

    // we need to convert the floats from network representation to host representation
    for (i = 0; i < full_array_length; i++) {
        full_array_as_ints[i] = ntohl(full_array_as_ints[i]);
        memcpy(&full_array[i], &full_array_as_ints[i], sizeof(float));
    }

    close(socket_fd);   // can close the socket now

    // print what we got
    printf("N = %d\n", full_array_length);
    printf("The array is:\n");
    for (i = 0; i < full_array_length; i++) {
        printf("%f ", full_array[i]);
    }
    printf("\n");

    return 0;
}
