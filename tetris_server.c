/*
Author for TCP Connection: Karl Thimm
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_PORT 5555
#define ROWS 20
#define COLS 15
#define TRUE 1
#define FALSE 0

char Table[ROWS][COLS] = {0};
int score = 0;
char GameOn = TRUE;
suseconds_t timer = 400000;
int decrease = 1000;

typedef struct {
    char **array;
    int width, row, col;
} Shape;
Shape current;

// Defines the shapes used in the game
const Shape ShapesArray[7] = {
    {(char *[]){(char[]){0, 1, 1}, (char[]){1, 1, 0}, (char[]){0, 0, 0}}, 3},
    {(char *[]){(char[]){1, 1, 0}, (char[]){0, 1, 1}, (char[]){0, 0, 0}}, 3},
    {(char *[]){(char[]){0, 1, 0}, (char[]){1, 1, 1}, (char[]){0, 0, 0}}, 3},
    {(char *[]){(char[]){0, 0, 1}, (char[]){1, 1, 1}, (char[]){0, 0, 0}}, 3},
    {(char *[]){(char[]){1, 0, 0}, (char[]){1, 1, 1}, (char[]){0, 0, 0}}, 3},
    {(char *[]){(char[]){1, 1}, (char[]){1, 1}}, 2},
    {(char *[]){(char[]){0, 0, 0, 0}, (char[]){1, 1, 1, 1}, (char[]){0, 0, 0, 0}, (char[]){0, 0, 0, 0}}, 4}
};

// Creates and returns a copy of a shape
Shape CopyShape(Shape shape) {
    Shape new_shape = shape;
    char **copyshape = shape.array;
    new_shape.array = (char **)malloc(new_shape.width * sizeof(char *));
    for (int i = 0; i < new_shape.width; i++) {
        new_shape.array[i] = (char *)malloc(new_shape.width * sizeof(char));
        for (int j = 0; j < new_shape.width; j++) {
            new_shape.array[i][j] = copyshape[i][j];
        }
    }
    return new_shape;
}

void DeleteShape(Shape shape) {
    for (int i = 0; i < shape.width; i++) {
        free(shape.array[i]);
    }
    free(shape.array);
}

int CheckPosition(Shape shape) {
    char **array = shape.array;
    for (int i = 0; i < shape.width; i++) {
        for (int j = 0; j < shape.width; j++) {
            if ((shape.col + j < 0 || shape.col + j >= COLS || shape.row + i >= ROWS)) {
                if (array[i][j]) return FALSE;
            } else if (Table[shape.row + i][shape.col + j] && array[i][j]) return FALSE;
        }
    }
    return TRUE;
}

void SetNewRandomShape() {
    Shape new_shape = CopyShape(ShapesArray[rand() % 7]);
    new_shape.col = rand() % (COLS - new_shape.width + 1);
    new_shape.row = 0;
    DeleteShape(current);
    current = new_shape;
    if (!CheckPosition(current)) {
        GameOn = FALSE;
    }
}

void RotateShape(Shape shape) {
    Shape temp = CopyShape(shape);
    int width = shape.width;
    for (int i = 0; i < width; i++) {
        for (int j = 0, k = width - 1; j < width; j++, k--) {
            shape.array[i][j] = temp.array[k][i];
        }
    }
    DeleteShape(temp);
}

void WriteToTable() {
    for (int i = 0; i < current.width; i++) {
        for (int j = 0; j < current.width; j++) {
            if (current.array[i][j])
                Table[current.row + i][current.col + j] = current.array[i][j];
        }
    }
}

void RemoveFullRowsAndUpdateScore()
{
	int i, j, sum, count = 0;
	for (i = 0; i < ROWS; i++)
	{
		sum = 0;
		for (j = 0; j < COLS; j++)
		{
			sum += Table[i][j];
		}
		if (sum == COLS)
		{
			count++;
			int l, k;
			for (k = i; k >= 1; k--)
				for (l = 0; l < COLS; l++)
					Table[k][l] = Table[k - 1][l];
			for (l = 0; l < COLS; l++)
				Table[k][l] = 0;
			timer -= decrease--;
		}
	}
	score += 100 * count;
}

void PrintTable()
{
	char Buffer[ROWS][COLS] = {0};
	int i, j;
	for (i = 0; i < current.width; i++)
	{
		for (j = 0; j < current.width; j++)
		{
			if (current.array[i][j])
				Buffer[current.row + i][current.col + j] = current.array[i][j];
		}
	}
	clear();
	for (i = 0; i < COLS - 9; i++)
		printw(" ");
	printw("Covid Tetris\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			printw("%c ", (Table[i][j] + Buffer[i][j]) ? '#' : '.');
		}
		printw("\n");
	}
	printw("\nScore: %d\n", score);
}

void ManipulateCurrent(char action)
{
	Shape temp = CopyShape(current);
	switch (action)
	{
	case 's':
		temp.row++; // move down
		if (CheckPosition(temp))
			current.row++;
		else
		{
			WriteToTable();
			RemoveFullRowsAndUpdateScore();
			SetNewRandomShape();
		}
		break;
	case 'd':
		temp.col++; // Moves blocks right
		if (CheckPosition(temp))
			current.col++;
		break;
	case 'a':
		temp.col--; // Moves Blocks left
		if (CheckPosition(temp))
			current.col--;
		break;
	case 'w':
		RotateShape(temp); // Rotate Blocks
		if (CheckPosition(temp))
			RotateShape(current);
		break;
	}
	DeleteShape(temp);
	PrintTable();
}

struct timeval before_now, now;
int hasToUpdate()
{
	return ((suseconds_t)(now.tv_sec * 1000000 + now.tv_usec) - ((suseconds_t)before_now.tv_sec * 1000000 + before_now.tv_usec)) > timer;
}

int main()
{
	srand(time(0));  // Initialize random seed based on current time for shape selection
	score = 0;       // Initialize the game score to 0

	// Networking variables
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer;  // Buffer to receive commands from the client

	// Socket creation for the TCP connection
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Attach socket to the port 5555 forcefully
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;           // Internet protocol (IPv4)
	address.sin_addr.s_addr = INADDR_ANY;   // Accept connections on any IP
	address.sin_port = htons(SERVER_PORT);         // Convert port number to network byte order

	// Bind the socket to the address and port number
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Listen for incoming connections
	if (listen(server_fd, 3) < 0)  // Backlog set to 3, the max number of pending connections
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// Accept a connection, creating a new socket for the connection
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	// Set the new socket to non-blocking mode to allow for continuous game updates
	int flags = fcntl(new_socket, F_GETFL, 0);
	fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

	// Initialize ncurses for graphical display in terminal
	initscr();  // Start curses mode
	gettimeofday(&before_now, NULL);  // Get the current time to manage game speed
	timeout(1);  // Set a short delay for keypresses, to make the game responsive

	SetNewRandomShape();  // Spawn the first shape in the game
	PrintTable();  // Print the initial game board

	// Game loop
	while (GameOn)
	{
		valread = recv(new_socket, &buffer, sizeof(buffer), 0);  // Non-blocking receive, check for input
		if (valread == 0)
		{
			printw("Client disconnected\n");  // Client has closed the connection
			break;
		}
		else if (valread < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				// No data received, continue game execution
				refresh();  // Update the screen with the latest game state
				gettimeofday(&now, NULL);  // Get current time to calculate if a game update is needed
				if (hasToUpdate())  // Check if it's time to move the shape down
				{
					ManipulateCurrent('s');  // Move the shape down automatically
					gettimeofday(&before_now, NULL);  // Reset the timer for the next move
				}
			}
			else
			{
				perror("recv failed");
				break;
			}
		}
		else
		{
			ManipulateCurrent(buffer);  // Process player input to manipulate the shape
			refresh();  // Redraw the screen after state change
		}
	}

	DeleteShape(current);  // Clean up memory for the current shape
	endwin();  // End curses mode, return to normal terminal state

	// Display the final game state and score
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			printf("%c ", Table[i][j] ? '#' : '.');
		}
		printf("\n");
	}
	printf("\nGame Over!\n");
	printf("\nScore: %d\n", score);

	return 0;
}

