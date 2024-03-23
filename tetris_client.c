/*
Author for TCP Connection: Karl Thimm
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_PORT 5555

int dataPacketsSent = 0; // Tracks the number of data packets sent
char isGameActive = TRUE; // Flag to control the game execution loop

// Function to transmit user's key press to the server
void sendKeyPress(int serverSocket, int keyPressed) {
    switch (keyPressed) {
    case 's':
    case 'd':
    case 'a':
    case 'w':
        clear(); // Refresh the screen
        send(serverSocket, &keyPressed, sizeof(keyPressed), 0); // Transmit key press to server
        printw("Data packet %d sent\n", ++dataPacketsSent); // Show packet count
        break;
    case 'q':
        clear(); // Refresh the screen
        printw("Q Pressed, Quitting Game");
        refresh();
        sleep(2); // Short delay to display message
        close(serverSocket); // Disconnect from the server
        isGameActive = FALSE; // Set the game loop to end
        break;
    }
}

int main() {
    int clientSocket = 0, readStatus;
    struct sockaddr_in serverAddress;
    int netBuffer;
    int userInputKey;

    // Set up a client socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    // Define server's address
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address format or unsupported address\n");
        return -1;
    }

    // Attempt to connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Unable to connect to server");
        return -1;
    }

    // Make the socket non-blocking
    int socketFlags = fcntl(clientSocket, F_GETFL, 0);
    fcntl(clientSocket, F_SETFL, socketFlags | O_NONBLOCK);

    initscr(); // Start ncurses mode
    cbreak(); // Direct input to the program without buffering
    noecho(); // Prevent echoing of user input

    // Begin the main game loop
    while (isGameActive) {
        if ((userInputKey = getch()) != ERR) { // Check for user input
            sendKeyPress(clientSocket, userInputKey); // Send user input to the server
        }

        // Check for incoming data from the server
        readStatus = recv(clientSocket, &netBuffer, sizeof(netBuffer), 0);
        if (readStatus == 0) {
            printw("Connection with server lost\n");
            break;
        } else if (readStatus < 0 && !(errno == EWOULDBLOCK || errno == EAGAIN)) {
            perror("Network read error");
            break;
        }
    }
    endwin(); // Close ncurses window
    printf("Total data packets sent: %d\n", dataPacketsSent); // Report total packets sent

    return 0;
}
