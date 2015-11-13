COMPILERFLAGS = -g -Wall -Wextra -Wno-sign-compare
CC = gcc

SERVEROBJECTS = sender_main.o log.o
CLIENTOBJECTS = receiver_main.o


.PHONY: all clean

all : reliable_sender reliable_receiver

reliable_sender: $(SERVEROBJECTS)
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

reliable_receiver: $(CLIENTOBJECTS)
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)
	
log.o: log.c log.h
	$(CC) -o $@ $< -c $(LINKLIBS)
	
clean :
	$(RM) reliable_sender.o reliable_receiver.o log.o sender_main.o receiver_main.o reliable_sender reliable_receiver
