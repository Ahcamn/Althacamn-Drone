#ifndef VAISSEAU_H_INCLUDED
#define VAISSEAU_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdbool.h>
#include <time.h>

typedef enum Type Type;
enum Type
{
	PETIT, MOYEN, GROS
};

#include "client.h"

#define NB_DRONE_PETIT 4
#define NB_DRONE_MOYEN 3
#define NB_DRONE_GROS  2

#define IN_MILLISECONDS 1000


/* Définition des Structures */

typedef struct DroneStruct
{
    int droneID;
    Type type;
    float batterie;
    float tempsRecharge;
    int ventMax;
    
}DroneStruct;

typedef DroneStruct* Drone;

typedef struct MeteoStruct
{
 bool temps_praticable;
 int vent;
}MeteoStruct;

typedef MeteoStruct* Meteo;


/* Variables globales */

Meteo meteo;

typedef struct Vaisseau
{
    int tempClientID;
    int clientsLivres;
    
    int nbClients;
    int nbPetitDrones;
    int nbMoyenDrones;
    int nbGrosDrones;
    
    Drone *dronesP;
    Drone *dronesM;
    Drone *dronesG;
    
    pthread_t *drone_petit;    
    pthread_t *drone_moyen;    
    pthread_t *drone_gros;   
    
    pthread_cond_t condition_drone;
    pthread_cond_t condition_client;
    pthread_mutex_t mutex; 
}Vaisseau;

/* Prototypes des fonctions */

void erreur(const char*);
void *fonc_client(void*); 
void *fonc_drone(void*); 

void createDroneThread(pthread_t*, int, Type);
Drone createDrone(Type, int);

bool canDeliver(int, float, int);
float recharger(float , Type, time_t*);
void generationMeteo();
const char* getTypeName(Type);

void allocateMemory();
void freeMemory();

#endif
