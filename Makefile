tetris: client server

client: tetris_client.c
	gcc -o tetris_client tetris_client.c -lncurses

server: tetris_server.c
	gcc -o tetris_server tetris_server.c -lncurses
	
clean:
	rm tetris_client tetris_server
