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
}

int main() {
    int sid = 0;
    char *send_data = (char *)malloc(1024);
    char *receive_data = (char *)malloc(1024);
    struct sockaddr_in server_socket;
    socklen_t size = sizeof(server_socket);

    if ((sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Connection error at client side");
        exit(1);
    }

    set_socket(&server_socket, AF_INET, 6000);

    if (inet_aton("127.0.0.1", &server_socket.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        free(send_data);
        free(receive_data);
        close(sid);
        exit(1);
    }

    // Additional input for output filename
    char output_filename[256];
    printf("Enter the name under which to save the file: ");
    scanf("%s", output_filename);

    // Request from user for the file name to send to server
    printf("Enter the name of the file you want to request: ");
    scanf("%s", send_data);

    // Send the filename request to the server
    sendto(sid, send_data, strlen(send_data), 0, (struct sockaddr *)&server_socket, size);

    // Open the file to write received data
    int fd = open(output_filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    if (fd == -1) {
        perror("File creation error");
        free(send_data);
        free(receive_data);
        close(sid);
        return 1;
    }

    // Receive data from the server
    while (1) {
        int bytes_received = recvfrom(sid, receive_data, 1024, 0, (struct sockaddr *)&server_socket, &size);

        if (bytes_received < 0) {
            perror("Receive error");
            break;
        }

        if (strncmp(receive_data, "ENDOFFILE", 9) == 0) {
            printf("End of file transmission.\n");
            break;
        } else {
            // Write the received data directly to the file
            write(fd, receive_data, bytes_received);
        }
    }

    close(fd);
    close(sid);
    free(send_data);
    free(receive_data);

    return 0;
}
