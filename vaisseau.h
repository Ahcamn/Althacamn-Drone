#ifndef VAISSEAU_H_INCLUDED
#define VAISSEAU_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/sem.h>
#include <sys/msg.h>
#include <pthread.h> 
#include <stdbool.h>
#include <time.h>

#include "client.h"

#define NB_DRONE_PETIT 6
#define NB_DRONE_MOYEN 4
#define NB_DRONE_GROS  2

#define PETIT 1
#define MOYEN 2
#define GROS  3
#define IN_MILLISECONDS 1000

typedef struct Vaisseau
{
    int nbPetitColis;
    int nbMoyenColis;
    int nbGrosColis;
    
    int tempClientID;
    int clientsLivres;
    
    pthread_t drone_petit[NB_DRONE_PETIT];    
    pthread_t drone_moyen[NB_DRONE_MOYEN];    
    pthread_t drone_gros[NB_DRONE_GROS];   
    
    pthread_cond_t condition_drone;
    pthread_cond_t condition_client;
    pthread_mutex_t mutex; 
}Vaisseau;

typedef struct DroneStruct
{
    int type;
    float autonomie;
    float tempsRecharge;
    bool disponibilite;
    
}DroneStruct;

typedef DroneStruct* Drone;

void erreur(const char*);
void *fonc_client(void*); 
void *fonc_droneP(void*);
// void *fonc_droneM(void*);
// void *fonc_droneG(void*);
Drone createDrone(int);
void createDroneThread(pthread_t*, int, int);
float recharger(float , int, time_t*);


#endif
