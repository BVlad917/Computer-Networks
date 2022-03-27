#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 4242

int main() {
    int socket_fd;
    struct sockaddr_in receiver;
    char send_buff[256], receive_buff[256];
    socklen_t receiver_size;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    int broadcast_yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (void*) &broadcast_yes, sizeof(broadcast_yes)) < 0) {
        perror("failed to set broadcast socket option");
        return -2;
    }

    memset(&receiver, 0, sizeof(receiver));
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT);
    receiver.sin_addr.s_addr = inet_addr("192.168.1.255");

    strcpy(send_buff, "i'm sending a broadcast message");
    if (sendto(socket_fd, (void*)send_buff, strlen(send_buff) + 1, 0, (struct sockaddr*) &receiver, sizeof(receiver)) < 0) {
        perror("failed to send broadcast message");
        return -3;
    }

    memset(receive_buff, 0, sizeof(receive_buff));
    receiver_size = sizeof(receiver);
    if (recvfrom(socket_fd, (void*)receive_buff, sizeof(receive_buff), 0, (struct sockaddr*) &receiver, &receiver_size) < 0) {
        perror("failed to receive confirmation message");
        return -4;
    }

    printf("%s\n", receive_buff);

    close(socket_fd);

    return 0;
}
