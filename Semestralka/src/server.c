// Server side C program to demonstrate Socket programming
#include "world.h"
#include "input.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#define PORT 8082
int main(int argc, char const* argv[])
{
    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";
    int sprava = 0;
    int sirka = 20;
    int dlzka = 20;
    int rezim = 1;
    int typ = 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR , &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket
         = accept(server_fd, (struct sockaddr*)&address,
                  &addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    read(new_socket, &sirka, sizeof(int)); 
    read(new_socket, &dlzka, sizeof(int)); 
    read(new_socket, &rezim, sizeof(int));
    read(new_socket, &typ, sizeof(int));
    printf("%d", rezim);
    printf("%d", typ);
    
    World world;
    world_init(&world , sirka , dlzka, rezim, typ);
    time_t start_time = time(NULL);
    //send(new_socket, &sirka, sizeof(int), 0);
    //send(new_socket, &dlzka, sizeof(int), 0);
    
    // Čítanie správ od klienta, kým správa hra beží
    while (!world.game_over) {
        time_t current_time = time(NULL);
        int elapsed_time = (int)(current_time - start_time);
        world_update(&world, sprava);
        usleep(125000);
        send(new_socket, &world.game_over , sizeof(bool), 0);
        for (int i = 0; i < world.height; i++) {
            send(new_socket, world.grid[i], world.width * sizeof(char), 0);
        }
        send(new_socket, &world.snake.length, sizeof(world.snake.length), 0);
        send(new_socket, &elapsed_time, sizeof(elapsed_time), 0);
        read(new_socket, &sprava, sizeof(int));

    }

    
    
    printf("Hra skončila. Ďakujeme za hranie!\n");

    // Odoslanie odpovede klientovi
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    // Closing the connected socket
    close(new_socket);
    close(server_fd);
    
    return 0;



}
