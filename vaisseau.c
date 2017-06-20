#include "vaisseau.h"

Vaisseau vaisseau =
{
   .nbPetitColis = 0,
   .nbMoyenColis = 0,
   .nbGrosColis = 0,
   
   .tempClientID = 0,
   .clientsLivres = 0,
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
        // usleep(500*IN_MILLISECONDS);
    }
        
    
    for(i=0; i < NB_DRONE_PETIT; i++)
       if(pthread_join(vaisseau.drone_petit[i], NULL))
            erreur("Erreur pthread_join Drone\n");
    
    for(i=0; i < NB_CLIENTS; i++)
        if(pthread_join(client[i], NULL))
            erreur("Erreur pthread_join Client\n");
      
    exit(EXIT_SUCCESS);
}



void *fonc_client(void *arg) 
{
    int clientID = (int)arg;
    clients[clientID] = createClient(clientID);
    Client c = clients[clientID];

    bool finLivraison = false;
    bool livrable = false;
    
    
    if(c->couvert && c->jardin && c->tempsTrajet <= 30)
    {
        // printf("Client %d éligible / tempsTrajet : %d minutes / colis : %d\n", clientID, c->tempsTrajet, c->order->type);
        livrable = true;
        pthread_mutex_lock(&vaisseau.mutex);
        vaisseau.nbPetitColis++;
        pthread_mutex_unlock(&vaisseau.mutex);
    }   
    else
        printf("\t\t\t\t\t\tClient %d non éligible.\n\n", clientID);
    
    
    while(livrable && c->satisfait && !finLivraison)
    {
        pthread_mutex_lock(&vaisseau.mutex);
        
        vaisseau.tempClientID = clientID;
        
        pthread_cond_signal(&vaisseau.condition_drone);
        pthread_cond_wait(&vaisseau.condition_client, &vaisseau.mutex);
                
        if(c->present)
        {            
            if(c->order->livre)
            {
                printf("Client %d livré.\n\n", clientID);
                finLivraison = true;
            }  
            else
            {
                printf("\t\t\tClient %d en attente.\n\n", clientID);
                sleep(1);
            }
        }
        else
        {
            printf("\t\t\t\t\t\tClient %d absent.\n\n", clientID);
            livrable = false;
        }

        pthread_mutex_unlock(&vaisseau.mutex);
    }
    
    vaisseau.clientsLivres++;
    if(vaisseau.clientsLivres == NB_CLIENTS)
        printf("Tous les clients ont été livrés !\n");
    
    pthread_exit(NULL);
}


void *fonc_droneP(void *arg) 
{
    int type = PETIT;
    int droneID = (int)arg;
    int clientID;
    time_t oldTime;
    time(&oldTime);
    Drone d = createDrone(type);
    Client c;
    
    
    while(1)
    {
        pthread_mutex_lock(&vaisseau.mutex);        
        pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        
        d->autonomie = recharger(d->autonomie, d->type, &oldTime);
        // printf("Drone %d reveillé par Client %d (Petit) / autonomie : %.1f\n", droneID, vaisseau.tempClientID, d->autonomie);
        
        clientID = vaisseau.tempClientID;
        c = clients[clientID];
        
        if(d->autonomie >= c->tempsTrajet)
        {                    
            vaisseau.nbPetitColis--;
            c->order->livre = true;
            d->autonomie -= c->tempsTrajet;
            
            // printf("Drone %d livre Client %d / Autonomie restante : %.1f minutes\n", droneID, c->clientID, d->autonomie); 
        }
       /* else
        {
            printf("Drone %d manque d'autonomie / %.1f < %d mns.\n", droneID, d->autonomie, c->tempsTrajet);
        }*/
        
        pthread_cond_signal(&vaisseau.condition_client);
        pthread_mutex_unlock(&vaisseau.mutex);   
    }   
          
    pthread_exit(NULL);
} 


/*void *fonc_droneM(void *arg) 
{
     int type = PETIT;
    int droneID = (int)arg;
    time_t oldTime;
    time(&oldTime);
    Drone d = createDrone(type);
    Client c;
    Order order;
    
    
    while(1)
    {
        pthread_mutex_lock(&vaisseau.mutex);        
        pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        
        d->autonomie = recharger(d->autonomie, d->type, &oldTime);
        // printf("Drone %d reveillé par Client %d (Petit) / autonomie : %.1f\n", droneID, vaisseau.tempClientID, d->autonomie);
        
        c = clients[vaisseau.tempClientID];
        order = c->order;
        
        pthread_mutex_unlock(&vaisseau.mutex);
        
        if(d->autonomie < c->tempsTrajet)
        {
            // printf("Drone %d manque d'autonomie / %.1f < %d mns.\n", droneID, d->autonomie, c->tempsTrajet);
            pthread_cond_signal(&vaisseau.condition_client);
        }
        else
        {
            pthread_mutex_lock(&vaisseau.mutex);
        
            vaisseau.nbPetitColis--;
            order->livre = true;
            d->autonomie -= c->tempsTrajet;
            
            // printf("Drone %d livre Client %d / Autonomie restante : %.1f minutes\n", droneID, c->clientID, d->autonomie);
            pthread_cond_signal(&vaisseau.condition_client);
            pthread_mutex_unlock(&vaisseau.mutex);
        }
    }   
          
    pthread_exit(NULL);
} 

void *fonc_droneG(void *arg) 
{
     int type = PETIT;
    int droneID = (int)arg;
    time_t oldTime;
    time(&oldTime);
    Drone d = createDrone(type);
    Client c;
    Order order;
    
    
    while(1)
    {
        pthread_mutex_lock(&vaisseau.mutex);        
        pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        
        d->autonomie = recharger(d->autonomie, d->type, &oldTime);
        // printf("Drone %d reveillé par Client %d (Petit) / autonomie : %.1f\n", droneID, vaisseau.tempClientID, d->autonomie);
        
        c = clients[vaisseau.tempClientID];
        order = c->order;
        
        pthread_mutex_unlock(&vaisseau.mutex);
        
        if(d->autonomie < c->tempsTrajet)
        {
            // printf("Drone %d manque d'autonomie / %.1f < %d mns.\n", droneID, d->autonomie, c->tempsTrajet);
            pthread_cond_signal(&vaisseau.condition_client);
        }
        else
        {
            pthread_mutex_lock(&vaisseau.mutex);
        
            vaisseau.nbPetitColis--;
            order->livre = true;
            d->autonomie -= c->tempsTrajet;
            
            // printf("Drone %d livre Client %d / Autonomie restante : %.1f minutes\n", droneID, c->clientID, d->autonomie);
            pthread_cond_signal(&vaisseau.condition_client);
            pthread_mutex_unlock(&vaisseau.mutex);
        }
    }   
          
    pthread_exit(NULL);
} */




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
                if(pthread_create(&drone[i], NULL, fonc_droneP, (void*)i))
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


float recharger(float autonomie, int type, time_t *oldTime)
{
    time_t now;
    time(&now);
    double recharge = (double)now - (double)*oldTime;
    *oldTime = now;
    float newAutonomie = autonomie;
    
    float maxAutonomie;
    switch(type)
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
    
    // printf("recharge = %f / autonomie = %f \n", recharge, d->autonomie);
    if((recharge*2 + autonomie) >= maxAutonomie)
        newAutonomie = maxAutonomie;
    else
        newAutonomie += recharge*2;
    
    return newAutonomie;
}
