EXECS= server client

all: $(EXECS)

server: server.c configs.h
		gcc -o edu_server server.c -lrt

client: client.c configs.h
		gcc -o edu_client client.c -lrt

clean:
		rm -f $(EXECS)