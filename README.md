This code uses the tetris_server to run the game on port 5555. The tetris_client is responsible for sending the keystroke
to the server using a TCP connection. 

Steps to Run:
1) Open Two Terminal Windows
2) CD to the project folder containing the client and server files.
3) Type "make" in the console
4) In one of the terminals run "./tetris_server (Make sure to do this first)
5) In the other terminal type "./tetris_client"
6) The board will be displayed in the server terminal and the keystrokes are sent by the terminal running the client.
7) Play Tetris!
8) To quit the game press "Q"
9) To remove the files running the game type "make clean"