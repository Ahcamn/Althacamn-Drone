CC = gcc
CFLAGS = -Wall
THREAD = -lpthread

all : vaisseau.o client.o
	$(CC) -o Drone $^ $(THREAD)

vaisseau.o : vaisseau.c vaisseau.h
	$(CC) $(CFLAGS) -c $<
    
client.o : client.c client.h
	$(CC) $(CFLAGS) -c $< 
    
clean : 
	rm -rf *.o
