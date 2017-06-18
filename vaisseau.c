#include "vaisseau.h"

void erreur(const char *msg)
{
    perror(msg);
}


int main()
{   
    int i;
    // int msgid;
    srand(time(NULL));
    
    /*if ((msgid = msgget(CLE, IPC_CREAT | IPC_EXCL | 0600)) == -1)
		erreur("Erreur creation file de messages");*/
    
    for(i=0; i< NB_CLIENTS; i++)
        createClientThread(ville.client[i], i);
    
    createDroneThread(vaisseau.drone_petit, NB_DRONE_PETIT, PETIT);
    // createDroneThread(vaisseau.drone_moyen, NB_DRONE_MOYEN, MOYEN);
    // createDroneThread(vaisseau.drone_gros, NB_DRONE_GROS, GROS);
    
    
    /*if(msgctl(msgid, IPC_RMID, 0) == -1)
		erreur("Erreur suppresion de la file de messages\n");*/
    
    exit(EXIT_SUCCESS);
}


void *fonc_droneP(void *arg) 
{
    int type = (int)arg;
    // int msgid;
    Drone d = createDrone(type);
    Client c;
    
    while(1)
    {
        pthread_mutex_lock(&vaisseau.mutex_drone);
        
        if(vaisseau.nbPetitColis == 0 || d->autonomie <= 0)
        {
            pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex_drone);
        }
        
        /*if ((msgid = msgget(CLE, 0)) == -1) 
            erreur("Ereur msgget Drone\n");
        
        if (msgrcv(msgid, &c, sizeof(Client), 1, 0) == -1)
            perror("Erreur de lecture requete \n");*/

        vaisseau.nbPetitColis--;
        //d->autonomie -= c->tempsTrajet*2;
		d->autonomie -= 40;
        printf("ID : %lu / Type : %d / Autonomie : %.1fh / Temps de recharge : %.1fh.\n", pthread_self(), type, d->autonomie, d->tempsRecharge);
        printf("Colis restant : %d\n", vaisseau.nbPetitColis);
        pthread_mutex_unlock(&vaisseau.mutex_drone);
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
                d->autonomie = 200;
                d->tempsRecharge = 60.0f;
                break;
            case MOYEN:
                d->autonomie = 300;
                d->tempsRecharge = 90.0f;
                break;
            case GROS:
                d->autonomie = 400;
                d->tempsRecharge = 120.0f;
                break;
        };
    }
    else
        erreur("Erreur création Drone\n");
    
	return d;
}

void createDroneThread(pthread_t *drone, int nbDrones, int type)
{
    int i;
    
    for(i=0; i<nbDrones; i++)
        if(pthread_create(&drone[i], NULL, fonc_droneP, (void*)type))
            erreur("Erreur création thread Drone\n");
    
    for(i=0; i<nbDrones; i++)
        if(pthread_join(drone[i], NULL))
            erreur("Erreur pthread_join Drone\n");
        
    printf("\n");
}
