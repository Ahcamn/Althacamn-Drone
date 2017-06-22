#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdbool.h>
#include <time.h>

#include "vaisseau.h"

#define NB_CLIENTS 15

/* Définition des Structures */

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
    bool enAttente;
    float tempsTrajet;
    Order order;
}ClientStruct;

typedef ClientStruct* Client;

/* Variables globales */

Client *clients; 
pthread_t *client;
 
 
/* Prototypes des fonctions */

void createClientThread(pthread_t*, int);
Client createClient(int);

Order createOrder();
bool alea();

#endif
