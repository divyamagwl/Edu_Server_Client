EXECS= server client

all: $(EXECS)

server: server.c
		gcc -o edu_server server.c -lrt

client: client.c
		gcc -o edu_client client.c -lrt

clean:
		rm -f $(EXECS)