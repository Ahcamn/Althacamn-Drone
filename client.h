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

typedef struct Ville
{
    pthread_t client[NB_CLIENTS];  
    
    pthread_cond_t condition_client;
    pthread_mutex_t mutex_client; 
}Ville;

static Ville ville =
{
   .mutex_client = PTHREAD_MUTEX_INITIALIZER,
   .condition_client = PTHREAD_COND_INITIALIZER,
};


typedef struct ClientStruct
{
    bool couvert;
    bool jardin;
    bool present;
    int tempsTrajet;
    // int meteo;
    int colis;
}ClientStruct;

typedef ClientStruct* Client;


void *fonc_client(void*);
Client createClient();
void createClientThread(pthread_t, int);
bool pileOuface();

#endif
