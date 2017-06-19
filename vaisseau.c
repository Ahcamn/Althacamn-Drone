#include "vaisseau.h"

static Vaisseau vaisseau =
{
   .nbPetitColis = 0,
   .nbMoyenColis = 0,
   .nbGrosColis = 0,
   .mutex = PTHREAD_MUTEX_INITIALIZER,
   .condition_drone = PTHREAD_COND_INITIALIZER,
   .condition_client = PTHREAD_COND_INITIALIZER,
};

void erreur(const char *msg)
{
    perror(msg);
}


int main()
{   
    int i;
    srand(time(NULL));
            
    createDroneThread(vaisseau.drone_petit, NB_DRONE_PETIT, PETIT);
    // createDroneThread(vaisseau.drone_moyen, NB_DRONE_MOYEN, MOYEN);
    // createDroneThread(vaisseau.drone_gros, NB_DRONE_GROS, GROS);
    
    for(i=0; i < NB_CLIENTS; i++)
    {
        createClientThread(client[i], i);
        printf("\n");
        sleep(1);
    }
        
    
    for(i=0; i < NB_DRONE_PETIT; i++)
       if(pthread_join(vaisseau.drone_petit[i], NULL))
            erreur("Erreur pthread_join Drone\n");
    
    for(i=0; i < NB_CLIENTS; i++)
        if(pthread_join(client[i], NULL))
            erreur("Erreur pthread_join Client\n");
      
    exit(EXIT_SUCCESS);
}


void *fonc_droneP(void *arg) 
{
    int type = (int)arg;
    time_t oldTime;
    time(&oldTime);
    Drone d = createDrone(type);
    
    
    while(1)
    {
        pthread_mutex_lock(&vaisseau.mutex);        
        pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        
        recharger(d, &oldTime);
        printf("Drone reveillé (Petit) -> ");
        
        Colis colis = vaisseau.colis;
        Client c = clients[colis->clientID];
        
        pthread_mutex_unlock(&vaisseau.mutex);
        
        if(d->autonomie <= 0 || d->autonomie < c->tempsTrajet)
        {
            printf("Drone déchargé.\n");
            pthread_cond_signal(&vaisseau.condition_client);
            // printf("autonomie = %f / ID : %lu\n", d->autonomie, pthread_self());
        }
        else
        {
            pthread_mutex_lock(&vaisseau.mutex);
            // printf("Drone livre colis.\n");
        
            vaisseau.nbPetitColis--;
            colis->livre = true;
            d->autonomie -= c->tempsTrajet;
            
            printf("ID : %lu / Type : %d / Autonomie : %.1f minutes / Temps de recharge : %.1f minutes.\n", pthread_self(), type, d->autonomie, d->tempsRecharge);
            pthread_cond_signal(&vaisseau.condition_client);
            pthread_mutex_unlock(&vaisseau.mutex);
        }
    }   
          
    pthread_exit(NULL);
} 


/*void *fonc_droneM(void *arg) 
{
    int type = (int)arg;
    Drone d = createDrone(type);
    
    
    while(1)
    {
        pthread_mutex_lock(&vaisseau.mutex);        
        pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        printf("Drone reveillé (Moyen) -> ");
        
        if(d->autonomie <= 0)
        {
            printf("Drone déchargé.\n");
            pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        }

        printf("Drone livre colis.\n");
        Colis colis = vaisseau.colis;
        Client c = clients[colis->clientID];
        vaisseau.nbPetitColis--;
		d->autonomie -= c->tempsTrajet;

        printf("ID : %lu / Type : %d / Autonomie : %.1f minutes / Temps de recharge : %.1f minutes.\n", pthread_self(), type, d->autonomie, d->tempsRecharge);
        printf("Colis restant : %d\n", vaisseau.nbPetitColis);
        pthread_cond_signal(&vaisseau.condition_client);
        pthread_mutex_unlock(&vaisseau.mutex);
    }   
          
    pthread_exit(NULL);
} 

void *fonc_droneG(void *arg) 
{
    int type = (int)arg;
    Drone d = createDrone(type);
    
    
    while(1)
    {
        pthread_mutex_lock(&vaisseau.mutex);        
        pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        printf("Drone reveillé (Gros) -> ");
        
        if(d->autonomie <= 0)
        {
            printf("Drone déchargé.\n");
            pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        }

        printf("Drone livre colis.\n");
        Colis colis = vaisseau.colis;
        Client c = clients[colis->clientID];
        vaisseau.nbPetitColis--;
		d->autonomie -= c->tempsTrajet;

        printf("ID : %lu / Type : %d / Autonomie : %.1f minutes / Temps de recharge : %.1f minutes.\n", pthread_self(), type, d->autonomie, d->tempsRecharge);
        printf("Colis restant : %d\n", vaisseau.nbPetitColis);
        pthread_cond_signal(&vaisseau.condition_client);
        pthread_mutex_unlock(&vaisseau.mutex);
    }   
          
    pthread_exit(NULL);
} */

void *fonc_client(void *arg) 
{
    int clientID = (int)arg;
    clients[clientID] = createClient(clientID);
    Client c = clients[clientID];;
    
    bool satisfait = false;
    bool eligible = false;
    
    
    if(c->couvert && c->jardin && c->present && c->tempsTrajet <= 30)
    {
        printf("Client %d éligible / tempsTrajet : %d minutes / colis : %d\n", clientID, c->tempsTrajet, c->colis);
        eligible = true;
        
        pthread_mutex_lock(&vaisseau.mutex);
        vaisseau.nbPetitColis++;
        vaisseau.colis = createColis(c->colis, c->clientID);
        pthread_mutex_unlock(&vaisseau.mutex);
    }   
    else
        printf("Client %d non éligible.\n", clientID);
    
    
    while(!satisfait && eligible)
    {
        pthread_mutex_lock(&vaisseau.mutex);
        
        if(pthread_cond_signal(&vaisseau.condition_drone) != 0)
            erreur("Erreur pthread_cond_signal Drone depuis Client");
        
        pthread_cond_wait(&vaisseau.condition_client, &vaisseau.mutex);
        
        Colis colisVaisseau = vaisseau.colis;
        if(colisVaisseau->livre)
        {
            printf("Client livré\n");
            satisfait = true;
        }  
        else
        {
            printf("Client non livré.\n");
            sleep(1);
        }
           
        
        pthread_mutex_unlock(&vaisseau.mutex);
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
                d->autonomie = 40.0f;
                d->tempsRecharge = 20.0f;
                break;
            case MOYEN:
                d->autonomie = 60.0f;
                d->tempsRecharge = 40.0f;
                break;
            case GROS:
                d->autonomie = 90.0f;
                d->tempsRecharge = 60.0f;
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
    
    switch(type)
    {
        case PETIT:
            for(i=0; i<nbDrones; i++)
                if(pthread_create(&drone[i], NULL, fonc_droneP, (void*)type))
                    erreur("Erreur création thread Drone (Petit)\n");
            break;
        case MOYEN:
            /*for(i=0; i<nbDrones; i++)
                if(pthread_create(&drone[i], NULL, fonc_droneM, (void*)type))
                    erreur("Erreur création thread Drone (Moyen)\n");*/
            break;
        case GROS:
            /*for(i=0; i<nbDrones; i++)
                if(pthread_create(&drone[i], NULL, fonc_droneG, (void*)type))
                    erreur("Erreur création thread Drone (Gros)\n");*/
            break;
    }
    
            
    printf("\n");
}

Colis createColis(int type, int clientID)
{
    Colis colis = malloc(sizeof(Colis));
    
    if(colis != NULL)
    {
        colis->type = type;
        colis->clientID = clientID;
        colis->livre = false;
    }

    return colis;
}

void recharger(Drone d, time_t *oldTime)
{
    time_t now;
    time(&now);
    double recharge = (double)now - (double)*oldTime;
    *oldTime = now;
    
    float maxAutonomie;
    switch(d->type)
    {
        case PETIT:
            maxAutonomie = 40.0f;
            break;
        case MOYEN:
            maxAutonomie = 60.0f;
            break;
        case GROS:
            maxAutonomie = 90.0f;
            break;
    }
    
    printf("recharge = %f / autonomie = %f \n", recharge, d->autonomie);
    if((recharge*10 + d->autonomie) >= maxAutonomie)
        d->autonomie = maxAutonomie;
    else
        d->autonomie += recharge*10;
}
