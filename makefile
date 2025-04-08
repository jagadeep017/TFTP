client_debug: client.c c_fun.c tftp.c
	gcc -g client.c c_fun.c tftp.c -o client
client: client.c c_fun.c tftp.c
	gcc client.c c_fun.c tftp.c -o client
server_debug: server.c s_fun.c tftp.c
	gcc -g server.c s_fun.c tftp.c -o server
server: server.c s_fun.c tftp.c
	gcc server.c s_fun.c tftp.c -o server
clean:
	rm *.o client server