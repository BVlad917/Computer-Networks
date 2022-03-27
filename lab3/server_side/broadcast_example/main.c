#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/unistd.h>

#define PORT 4242

int main() {
    int socket_fd, sender_size;
    struct sockaddr_in receiver, sender;
    char receive_buff[256], send_buff[256];

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("failed to create socket");
        return -1;
    }

    int broadcast_yes = 1, reuse_yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (void*) &broadcast_yes, sizeof(broadcast_yes)) < 0) {
        perror("failed to set broadcast socket option");
        return -2;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*) &reuse_yes, sizeof(reuse_yes)) < 0) {
        perror("failed to set reuse socket option");
        return -3;
    }

    memset(&receiver, 0, sizeof(receiver));
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT);
    receiver.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr*) &receiver, sizeof(receiver)) < 0) {
        perror("failed to bind");
        return -4;
    }

    printf("Listening...\n");
    memset(receive_buff, 0, sizeof(receive_buff));
    sender_size = sizeof(sender);
    if (recvfrom(socket_fd, (void*)receive_buff, sizeof(receive_buff), 0, (struct sockaddr*) &sender, &sender_size) < 0) {
        perror("failed to receive broadcast message");
        return -5;
    }

    printf("%s\n", receive_buff);
    printf("From: %s:%d\n", inet_ntoa(sender.sin_addr), sender.sin_port);

    strcpy(send_buff, "got your message");
    if (sendto(socket_fd, (void*)send_buff, sizeof(send_buff), 0, (struct sockaddr*) &sender, sizeof(sender)) < 0) {
        perror("failed to send confirmation message");
        return -6;
    }

    close(socket_fd);

    return 0;
}
