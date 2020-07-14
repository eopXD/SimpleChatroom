
CC = gcc
DEL = rm -rf
all:
		$(DEL) asset
		$(DEL) bin
		$(DEL) file
		
		mkdir asset
		touch asset/userprofile
		mkdir bin
		mkdir file
		
		$(CC) src/linker.c -c
		$(CC) src/server.c linker.o -o bin/chat_server
		$(CC) src/client.c linker.o -o bin/chat_client
		$(DEL) *~
		$(DEL) *.o
clean:
		$(DEL) asset
		$(DEL) bin
		$(DEL) file
