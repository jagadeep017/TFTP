main: client.c c_fun.c tftp.c server.c s_fun.c
	gcc client.c c_fun.c tftp.c -o client/client
	gcc server.c s_fun.c tftp.c -o server/server
client_debug: client.c c_fun.c tftp.c
	gcc -g client.c c_fun.c tftp.c -o client/client
client: client.c c_fun.c tftp.c
	gcc client.c c_fun.c tftp.c -o client/client
server_debug: server.c s_fun.c tftp.c
	gcc -g server.c s_fun.c tftp.c -o server/server
server: server.c s_fun.c tftp.c
	gcc server.c s_fun.c tftp.c -o server/server
clean:
	rm -rf client/client server/server
debug: client.c c_fun.c tftp.c server.c s_fun.c
	gcc -g client.c c_fun.c tftp.c -o client/client
	gcc -g server.c s_fun.c tftp.c -o server/server