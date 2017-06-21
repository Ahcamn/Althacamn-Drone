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
    meteo = generationMeteo();
    srand(time(NULL));
            
    createDroneThread(vaisseau.drone_petit, NB_DRONE_PETIT, PETIT);
    createDroneThread(vaisseau.drone_moyen, NB_DRONE_MOYEN, MOYEN);
    createDroneThread(vaisseau.drone_gros, NB_DRONE_GROS, GROS);
    
    for(i=0; i < NB_CLIENTS; i++)
    {
        createClientThread(client[i], i);
        // usleep(100*IN_MILLISECONDS);
    }
    
    while(vaisseau.clientsLivres != NB_CLIENTS)
    {
        meteo = generationMeteo();
        usleep(500*IN_MILLISECONDS);
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
        modNbColis(c->order->type, 1);
        pthread_mutex_unlock(&vaisseau.mutex);
    }   
    else
        printf("\t\t\t\t\t\tClient %d non éligible.\n\n", clientID);
        
    while(livrable && c->satisfait && !finLivraison)
    {
        /* DEBUT section critique */
        pthread_mutex_lock(&vaisseau.mutex);
        
        vaisseau.tempClientID = clientID;
        
        /* Reveil du Drone */
		pthread_cond_signal(&vaisseau.condition_drone);
        /* Client mis en attente */
        pthread_cond_wait(&vaisseau.condition_client, &vaisseau.mutex);
                
        if(c->present)
        {            
            if(c->order->livre)
            {
                printf("Client %d livré.\n\n", clientID);
                finLivraison = true;
            }
            else if(!c->satisfait)
            {
                printf("\t\t\t\t\t\tClient %d non satisfait : retour colis\n\n", clientID);
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

        /* FIN section critique */
        pthread_mutex_unlock(&vaisseau.mutex);
    }
    
    vaisseau.clientsLivres++;
    if(vaisseau.clientsLivres == NB_CLIENTS)
        printf("Tous les clients livrables ont été livrés !\n");
   
    pthread_exit(NULL);
}


void *fonc_droneP(void *arg) 
{
    drone(PETIT, (int)arg);          
    pthread_exit(NULL);
} 

void *fonc_droneM(void *arg) 
{
    drone(MOYEN, (int)arg);          
    pthread_exit(NULL);
} 

void *fonc_droneG(void *arg) 
{
    drone(GROS, (int)arg);          
    pthread_exit(NULL);
}


void drone(Type type, int droneID)
{
    int clientID;
    time_t oldTime;
    time(&oldTime);
    Drone d = createDrone(type);
    Client c;
        
    while(1)
    {
        /* DEBUT section critique */
        pthread_mutex_lock(&vaisseau.mutex);
        
        /* Drone mis en attente */
        pthread_cond_wait(&vaisseau.condition_drone, &vaisseau.mutex);
        
        d->autonomie = recharger(d->autonomie, d->type, &oldTime);
        
        
        clientID = vaisseau.tempClientID;
        c = clients[clientID];
        
        // printf("Drone %d (%s) reveillé par Client %d (%s) / autonomie : %.1f / trajet : %d\n", droneID, getTypeName(d->type), clientID, getTypeName(c->order->type), d->autonomie, c->tempsTrajet);
        
        if(d->type == c->order->type)
        {
            if(meteo->temps_praticable && meteo->vent <= d->ventMax)
            {
                if(d->autonomie >= c->tempsTrajet)
                {    
                    c->satisfait = alea();
                    if(c->satisfait)
                    {
                        modNbColis(d->type, -1);
                        // printf("Client %d livraison du drone\n",clientID);
                        c->order->livre = true;
                    }
                    
                    d->autonomie -= c->tempsTrajet;
                    
                    printf("Drone %d (type %s) livre Client %d (Colis %s) / Autonomie restante : %.1f minutes\n", droneID, getTypeName(d->type), c->clientID, getTypeName(c->order->type), d->autonomie); 
                }
                /*else
                    printf("Drone %d manque d'autonomie pour Client %d / %.1f < %d mns.\n", droneID, clientID, d->autonomie, c->tempsTrajet);*/ 
            }
            else
                printf("\t\t\tMétéo défavorable, le drone ne part pas.\n");
        }
                
        /* Réveil du client */
        pthread_cond_signal(&vaisseau.condition_client);
        /* FIN section critique */
        pthread_mutex_unlock(&vaisseau.mutex);   
    }   
}

Drone createDrone(Type type)
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
                d->ventMax = 60;
                break;
            case MOYEN:
                d->autonomie = 60.0f;
                d->tempsRecharge = 40.0f;
                d->ventMax = 70;
                break;
            case GROS:
                d->autonomie = 90.0f;
                d->tempsRecharge = 60.0f;
                d->ventMax = 80;
                break;
        };
    }
    else
        erreur("Erreur création Drone\n");
    
	return d;
}

void createDroneThread(pthread_t *drone, int nbDrones, Type type)
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
            for(i=0; i<nbDrones; i++)
                if(pthread_create(&drone[i], NULL, fonc_droneM, (void*)i))
                    erreur("Erreur création thread Drone (Moyen)\n");
            break;
        case GROS:
            for(i=0; i<nbDrones; i++)
                if(pthread_create(&drone[i], NULL, fonc_droneG, (void*)i))
                    erreur("Erreur création thread Drone (Gros)\n");
            break;
    }
    
            
    printf("\n");
}


float recharger(float autonomie, Type type, time_t *oldTime)
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
            maxAutonomie = 50.0f;
            break;
        case GROS:
            maxAutonomie = 60.0f;
            break;
    }
    
    // printf("recharge = %f / autonomie = %f \n", recharge, d->autonomie);
    if((recharge*2 + autonomie) >= maxAutonomie)
        newAutonomie = maxAutonomie;
    else
        newAutonomie += recharge*2;
    
    return newAutonomie;
}

Meteo generationMeteo()
{
    Meteo meteo = malloc(sizeof(Meteo));
    if((rand()%10 + 1) == 1) 
        meteo->temps_praticable = false;
    else 
        meteo->temps_praticable = true;
    
    switch(rand()%5+1)
    {
        case 1:
        meteo->vent = rand()%20 + 1;
        break;
        case 2:
        meteo->vent = rand()%30 + 20;
        break;
        case 3:
        meteo->vent = rand()%40 + 30;
        break;
        case 4:
        meteo->vent = rand()%65 + 40;
        break;
        case 5:
        meteo->vent = rand()%100 + 65;
        break;
    }
    
    return meteo;
}

const char* getTypeName(Type type) 
{
   switch (type) 
   {
      case PETIT: return "Petit";
      case MOYEN: return "Moyen";
      case GROS: return "Gros";
   }
   exit(EXIT_FAILURE);
}

void modNbColis(Type type, int x)
{
    switch(type)
    {
        case PETIT:
            vaisseau.nbPetitColis += x;
            break;
        case MOYEN:
            vaisseau.nbMoyenColis += x;
            break;
        case GROS:
            vaisseau.nbGrosColis += x;
            break;
    }
}
