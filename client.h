#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/msg.h>
#include <pthread.h> 
#include <stdbool.h>

#include "vaisseau.h"

#define NB_CLIENTS 15



typedef struct OrderStruct
{
    Type type;
    bool livre;
}OrderStruct;

typedef OrderStruct* Order;

typedef struct ClientStruct
{
    int clientID;
    bool couvert;
    bool jardin;
    bool present;
    bool satisfait;
    int tempsTrajet;
    Order order;
}ClientStruct;

typedef ClientStruct* Client;

Client clients[NB_CLIENTS]; 

pthread_t client[NB_CLIENTS];
 
void createClientThread(pthread_t, int);
Client createClient(int);
Order createOrder();
bool alea();

#endif
