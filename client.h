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

#define NB_CLIENTS 10

typedef struct ClientStruct
{
    int clientID;
    bool couvert;
    bool jardin;
    bool present;
    int tempsTrajet;
    // int meteo;
    int colis;
}ClientStruct;

typedef ClientStruct* Client;

Client clients[NB_CLIENTS]; 

pthread_t client[NB_CLIENTS];
    

Client createClient(int);
void createClientThread(pthread_t, int);
bool alea();

#endif
