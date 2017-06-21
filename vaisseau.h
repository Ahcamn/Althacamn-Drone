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

typedef struct DroneStruct
{
    int droneID;
    Type type;
    float autonomie;
    float tempsRecharge;
    int ventMax;
    bool disponibilite;
    
}DroneStruct;

typedef DroneStruct* Drone;

typedef struct Vaisseau
{
    int nbPetitColis;
    int nbMoyenColis;
    int nbGrosColis;
    
    int tempClientID;
    int clientsLivres;
    
    Drone dronesP[NB_DRONE_PETIT];
    Drone dronesM[NB_DRONE_PETIT];
    Drone dronesG[NB_DRONE_PETIT];
    
    pthread_t drone_petit[NB_DRONE_PETIT];    
    pthread_t drone_moyen[NB_DRONE_MOYEN];    
    pthread_t drone_gros[NB_DRONE_GROS];   
    
    pthread_cond_t condition_drone;
    pthread_cond_t condition_client;
    pthread_mutex_t mutex; 
}Vaisseau;

typedef struct MeteoStruct
{
 bool temps_praticable;
 int vent;
}MeteoStruct;

typedef MeteoStruct* Meteo;

Meteo meteo;

void erreur(const char*);
void *fonc_client(void*); 
void *fonc_drone(void*);
void drone(Type, int); 
Drone createDrone(Type, int);
void createDroneThread(pthread_t*, int, Type);
bool canDeliver(int, int, int);
float recharger(float , Type, time_t*);
void generationMeteo();
const char* getTypeName(Type);
void modNbColis(Type, int);

#endif
