#ifndef VAISSEAU_H_INCLUDED
#define VAISSEAU_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/sem.h>
#include <pthread.h> 
#include <stdbool.h>

#include "client.h"


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