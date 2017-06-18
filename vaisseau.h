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

#define CLE 314

#define NB_DRONE_PETIT 6
#define NB_DRONE_MOYEN 4
#define NB_DRONE_GROS  2

#define PETIT 1
#define MOYEN 2
#define GROS  3

typedef struct Vaisseau
{
    int nbPetitColis;
    int nbMoyenColis;
    int nbGrosColis;


    pthread_t drone_petit[NB_DRONE_PETIT];    
    pthread_t drone_moyen[NB_DRONE_MOYEN];    
    pthread_t drone_gros[NB_DRONE_GROS];   
    
    pthread_cond_t condition_drone;
    pthread_mutex_t mutex_drone; 
}Vaisseau;


static Vaisseau vaisseau =
{
   .nbPetitColis = 10,
   .nbMoyenColis = 6,
   .nbGrosColis = 4,
   .mutex_drone = PTHREAD_MUTEX_INITIALIZER,
   .condition_drone = PTHREAD_COND_INITIALIZER,
};


typedef struct DroneStruct
{
    int type;
    float autonomie;
    float tempsRecharge;
    bool disponibilite;
    
}DroneStruct;

typedef DroneStruct* Drone;

typedef struct ColisStruct
{
    int type;
    int numClient;
}ColisStruct;

typedef ColisStruct* Colis;

void erreur(const char*);
void *fonc_droneP(void*);
Drone createDrone(int);
void createDroneThread(pthread_t*, int, int);

#endif
