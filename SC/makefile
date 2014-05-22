main: client.out server.out


client.out: client.cxx tmp.h
	g++ client.cxx -o client.out

server.out: server.cxx tmp.h
	g++ server.cxx -o server.out

clean:
	rm *.out

