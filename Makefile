TARGET = client server
CFLAGS = -Wall
normal: $(TARGET)
client: Client/client.c 
	gcc -Wall Client/client.c -lm -o client
server: Server/servlib.c Server/server.c
	gcc -Wall Server/servlib.c Server/server.c -lm -o server
clean:
	$(RM) $(TARGET)

