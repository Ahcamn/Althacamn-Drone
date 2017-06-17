
CC = gcc
CFLAGS = -Wall
THREAD = -lpthread

all : vaisseau.o
	$(CC) -o Drone $< $(THREAD)

vaisseau.o : vaisseau.c vaisseau.h
	$(CC) $(CFLAGS) -c $<
    
clean : 
	rm -rf *.o
