COMPILERFLAGS = -g -Wall -Wextra -Wno-sign-compare
CC = gcc

SERVEROBJECTS = distvec.o
CLIENTOBJECTS = linkstate.o


.PHONY: all clean

all : distvec linkstate

distvec: $(SERVEROBJECTS)
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

linkstate: $(CLIENTOBJECTS)
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

clean :
	$(RM) distvec.o linkstate.o distvec linkstate
