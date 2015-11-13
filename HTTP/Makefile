COMPILERFLAGS = -g -Wall -Wextra -Wno-sign-compare


SERVEROBJECTS = http_server.o
CLIENTOBJECTS = http_client.o


.PHONY: all clean

all : http_server http_client

http_server: $(SERVEROBJECTS)
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

http_client: $(CLIENTOBJECTS)
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)
	
clean :
	$(RM) http_server.o http_client.o
