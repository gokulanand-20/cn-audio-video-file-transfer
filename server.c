#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

void set_socket(struct sockaddr_in *socket, int type, int host_short) {
    socket->sin_family = type;
    socket->sin_port = htons(host_short);
    socket->sin_addr.s_addr = htonl(INADDR_ANY);
}

int main() {
    int sid = 0, fd;
    char *temp = (char *)malloc(500); // Buffer for reading the file
    char *receive_data = (char *)malloc(1024);
    struct sockaddr_in server_socket, client_socket;
    socklen_t size = sizeof(client_socket);

    if ((sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Socket creation error");
        free(temp);
        free(receive_data);
        exit(1);
    }

    set_socket(&server_socket, AF_INET, 6000);

    if (bind(sid, (struct sockaddr *)&server_socket, sizeof(server_socket)) == -1) {
        perror("Binding error");
        free(temp);
        free(receive_data);
        close(sid);
        exit(1);
    }

    printf("Server is waiting for client...\n");

    // Receive the filename from the client
    recvfrom(sid, receive_data, 1024, 0, (struct sockaddr *)&client_socket, &size);
    printf("Received request for file: %s\n", receive_data);
    fd = open(receive_data, O_RDONLY);

    if (fd == -1) {
        perror("File open error");
        free(temp);
        free(receive_data);
        close(sid);
        return 1;
    }

    // Send file data to client
    int count;
    while ((count = read(fd, temp, 500)) > 0) {
        printf("Sending data chunk of size %d...\n", count);
        sendto(sid, temp, count, 0, (struct sockaddr *)&client_socket, size);
    }

    // Indicate end of file transfer
    const char *end_signal = "ENDOFFILE";
    sendto(sid, end_signal, strlen(end_signal), 0, (struct sockaddr *)&client_socket, size);

    printf("File transfer complete.\n");

    close(fd);
    close(sid);
    free(temp);
    free(receive_data);
    return 0;
}
