EXECS= server client

all: $(EXECS)

server: server.c configs.h
		gcc -o server server.c -lrt -lpthread

client: client.c configs.h
		gcc -o client client.c -lrt

clean:
		rm -f $(EXECS)