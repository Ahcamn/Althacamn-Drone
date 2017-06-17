#include "vaisseau.h"

#define NB_DRONE_PETIT 6
#define NB_DRONE_MOYEN 4
#define NB_DRONE_GROS  2

#define PETIT 1
#define MOYEN 2
#define GROS  3


pthread_cond_t condition = PTHREAD_COND_INITIALIZER; /* Création de la condition */
pthread_mutex_t mutex_drone = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex */

static int nbPetitColis;
// static int nbMoyenColis;
// static int nbGrosColis;


void erreur(const char *msg)
{
    perror(msg);
}


int main()
{
    nbPetitColis = 10;
    
    pthread_t drone_petit[NB_DRONE_PETIT];    
    pthread_t drone_moyen[NB_DRONE_MOYEN];    
    pthread_t drone_gros[NB_DRONE_GROS];    
    
    createDroneThread(drone_petit, NB_DRONE_PETIT, PETIT);
    createDroneThread(drone_moyen, NB_DRONE_MOYEN, MOYEN);
    createDroneThread(drone_gros, NB_DRONE_GROS, GROS);
        
    printf("Création des Drones terminées !");
    exit(EXIT_SUCCESS);
}


void *fonc_droneP(void *arg) 
{
    int type = (int)arg;
    Drone d = createDrone(type);
    
    while(1)
    {
        pthread_mutex_lock(&mutex_drone);
        
        if(nbPetitColis == 0 || d->autonomie <= 0)
        {
            pthread_cond_wait(&condition, &mutex_drone);
        }

        nbPetitColis--;
        d->autonomie -= 1;
        printf("Type : %d / Autonomie : %.1fh / Temps de recharge : %.1fh.\n", type, d->autonomie, d->tempsRecharge);
        printf("Colis restant : %d\n", nbPetitColis);
        pthread_mutex_unlock(&mutex_drone);
    }
          
    pthread_exit(NULL);
} 


Drone createDrone(int type)
{
	Drone d = malloc(sizeof(Drone));
	if(d!=NULL)
    {
        d->type = type;
        d->disponibilite = true;
        
        switch(type)
        {
            case PETIT:
                d->autonomie = 2;
                d->tempsRecharge = 0.5f;
                break;
            case MOYEN:
                d->autonomie = 4;
                d->tempsRecharge = 1.0f;
                break;
            case GROS:
                d->autonomie = 6;
                d->tempsRecharge = 2.0f;
                break;
        };
    }
    else
        erreur("Erreur création Drone");
    
	return d;
}

void createDroneThread(pthread_t *drone, int nbDrones, int type)
{
    int i;
    for(i=0; i<nbDrones; i++)
    {
        if(pthread_create(&drone[i], 0, fonc_droneP, (void*)type))
            erreur("Erreur création thread");

        if(pthread_join(drone[i], NULL))
            erreur("Erreur pthread_join");
    }
    printf("\n");
}