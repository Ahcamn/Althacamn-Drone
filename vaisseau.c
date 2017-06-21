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
    generationMeteo();
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
        generationMeteo();
        usleep(1000*IN_MILLISECONDS);
    }
    
    for(i=0; i < NB_CLIENTS; i++)
        pthread_join(client[i], NULL);
        
    for(i=0; i < NB_DRONE_PETIT; i++)
       pthread_join(vaisseau.drone_petit[i], NULL);
        
    for(i=0; i < NB_DRONE_MOYEN; i++)
       pthread_join(vaisseau.drone_moyen[i], NULL);
    
    for(i=0; i < NB_DRONE_GROS; i++)
       pthread_join(vaisseau.drone_gros[i], NULL);
          
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
                if(!c->enAttente)
                {
                    c->enAttente = true;
                    printf("\t\t\tClient %d en attente.\n\n", clientID);
                }
                usleep(100*IN_MILLISECONDS);
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

void *fonc_drone(void *arg) 
{
    Drone d = (Drone)arg;
    int clientID;
    time_t oldTime;
    time(&oldTime);
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
                
        if(canDeliver(c, d))
        {
            // printf("Drone %d (%s) reveillé par Client %d (%s) / autonomie : %.1f / trajet : %d\n", d->droneID, getTypeName(d->type), clientID, getTypeName(c->order->type), d->autonomie, c->tempsTrajet);

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
                    
                    // printf("Drone %d (type %s) livre Client %d (Colis %s) / Autonomie restante : %.1f minutes\n\n", d->droneID, getTypeName(d->type), c->clientID, getTypeName(c->order->type), d->autonomie); 
                }
                /*else
                    printf("Drone %d manque d'autonomie pour Client %d / %.1f < %d mns.\n", d->droneID, clientID, d->autonomie, c->tempsTrajet);*/ 
            }
            else
                printf("\t\t\tClient %d en attente (Météo)\n\n", clientID);
        }
                
        /* Réveil du client */
        pthread_cond_signal(&vaisseau.condition_client);
        /* FIN section critique */
        pthread_mutex_unlock(&vaisseau.mutex);   
    }       
    pthread_exit(NULL);
} 

Drone createDrone(Type type, int i)
{
	Drone d = malloc(sizeof(Drone));
	if(d!=NULL)
    {
        d->type = type;
        d->disponibilite = true;
        
        switch(type)
        {
            case PETIT:
                d->droneID = 100 + i;
                d->autonomie = 40.0f;
                d->tempsRecharge = 20.0f;
                d->ventMax = 60;
                break;
            case MOYEN:
                d->droneID = 200 + i;
                d->autonomie = 60.0f;
                d->tempsRecharge = 40.0f;
                d->ventMax = 70;
                break;
            case GROS:
                d->droneID = 300 + i;
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
    
    for(i=0; i<nbDrones; i++)
    {
        Drone d = createDrone(type, i);
        switch(type)
        {
            case PETIT:
                vaisseau.dronesP[i] = d;
                break;
            case MOYEN:
                vaisseau.dronesM[i] = d;
                break;
            case GROS:
                vaisseau.dronesG[i] = d;
                break;
        }
        if(pthread_create(&drone[i], NULL, fonc_drone, (void*)d))
            erreur("Erreur création thread Drone (Petit)\n");
    }
}


bool canDeliver(Client c, Drone d)
{
    int i;
    bool deliver = false;
    
    if(c->order->type == d->type)
    {
        deliver = true;
    }
    else
    {
        if(c->order->type < d->type)
        {
            deliver = true;
            switch(c->order->type)
            {
                case PETIT:
                    
                    if(d->type == GROS)
                    {
                        for(i=0; i<NB_DRONE_MOYEN; i++)
                        {
                            Drone d = vaisseau.dronesM[i];
                            if(c->tempsTrajet < d->autonomie)
                                deliver = false;
                        }   
                    }
                    
                    for(i=0; i<NB_DRONE_PETIT; i++)
                    {
                        Drone d = vaisseau.dronesP[i];
                        if(c->tempsTrajet < d->autonomie)
                            deliver = false;
                    }
                    break;
                case MOYEN:
                    for(i=0; i<NB_DRONE_MOYEN; i++)
                    {
                        Drone d = vaisseau.dronesM[i];
                        if(c->tempsTrajet < d->autonomie)
                            deliver = false;
                    } 
                    break;
                case GROS:
                    break;
            }
        }
    }
    
    return deliver;
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
    
    if((recharge*2 + autonomie) >= maxAutonomie)
        newAutonomie = maxAutonomie;
    else
        newAutonomie += recharge*2;
    
    return newAutonomie;
}

void generationMeteo()
{
    if(meteo == NULL)
        meteo = malloc(sizeof(Meteo));
    
    if(rand()%10) 
        meteo->temps_praticable = true;
    else 
        meteo->temps_praticable = false;
    
    switch(rand()%5+1)
    {
        case 1:
        meteo->vent = rand()%20 + 1;
        break;
        case 2:
        meteo->vent = rand()%20 + 21;
        break;
        case 3:
        meteo->vent = rand()%20 + 41;
        break;
        case 4:
        meteo->vent = rand()%20 + 61;
        break;
        case 5:
        meteo->vent = rand()%20 + 81;
        break;
    }
        
    printf("------------------------------------------------------------------------------\n");
    if(meteo->temps_praticable)
        printf("\t\tChangement de Météo (Praticable / Vent : %d)\n", meteo->vent);
    else
        printf("\t\tChangement de Météo (Non praticable)\n");
    printf("------------------------------------------------------------------------------\n\n");
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
