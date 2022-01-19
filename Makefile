all:  server client

server: 
	gcc server.c -o userver

client: 
	gcc client.c -o uclient 

clean: 
	rm -rf userver uclient

