#include "vaisseau.h"

Vaisseau vaisseau =
{
   .tempClientID = 0,
   .clientsLivres = 0,
   .mutex = PTHREAD_MUTEX_INITIALIZER,
   .condition_drone = PTHREAD_COND_INITIALIZER,
   .condition_client = PTHREAD_COND_INITIALIZER,
};

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
        usleep(100*IN_MILLISECONDS);
    }
    
    while(vaisseau.clientsLivres != NB_CLIENTS)
    {
        generationMeteo();
        usleep(1000*IN_MILLISECONDS);
    }
    
    for(i=0; i < NB_CLIENTS; i++)
        pthread_join(client[i], NULL);
    
    printf("Tous les clients livrables ont été livrés !\n");
                  
    exit(EXIT_SUCCESS);
}

/* Fonction utilisée par les threads Client */
void *fonc_client(void *arg) 
{
    int clientID = (int)arg;
    clients[clientID] = createClient(clientID);
    Client c = clients[clientID];
    
    bool finLivraison = false;
    bool livrable = false;
    
    if(c->couvert && c->jardin && c->tempsTrajet <= 30)
        livrable = true;  
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
    pthread_exit(NULL);
}

/* Fonction utilisée par les threads DronesP, DronesM et DronesG */
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
        
        d->batterie = recharger(d->batterie, d->type, &oldTime);
        
        clientID = vaisseau.tempClientID;
        c = clients[clientID];
        
        if(canDeliver(c->order->type, c->tempsTrajet*2, d->type))     
        { 
            if(meteo->temps_praticable && meteo->vent <= d->ventMax)
            {
                // printf("Drone %d (%s) reveillé par Client %d (%s) / Batterie : %.1f / trajet : %d\n", d->droneID, getTypeName(d->type), clientID, getTypeName(c->order->type), d->batterie, c->tempsTrajet*2);
                if(d->batterie >= c->tempsTrajet*2)
                {    
                    c->enAttente = false;
                    c->satisfait = alea();
                    if(c->satisfait)
                        c->order->livre = true;
                    
                    float coef = 1.0f + c->order->type/20.0f;                    
                    d->batterie -= c->tempsTrajet*2*coef;
                    
                    // printf("Drone %d (%s) livre Client %d (%s) / Batterie restante : %.1f minutes / trajet : %d\n\n", d->droneID, getTypeName(d->type), c->clientID, getTypeName(c->order->type), d->batterie, c->tempsTrajet*2); 
                }
                /*else
                    printf("Drone %d manque de batterie pour Client %d / %.1f < %d mns.\n", d->droneID, clientID, d->batterie, c->tempsTrajet*2); */
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

/* Crée les différents threads de Drones en fonction de leur type */
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
            perror("Erreur création thread Drone (Petit)\n");
    }
}

/* Crée et initialise la structure d'un drone */
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
                d->batterie = 65.0f;
                d->tempsRecharge = 20.0f;
                d->ventMax = 60;
                break;
            case MOYEN:
                d->droneID = 200 + i;
                d->batterie = 75.0f;
                d->tempsRecharge = 40.0f;
                d->ventMax = 70;
                break;
            case GROS:
                d->droneID = 300 + i;
                d->batterie = 85.0f;
                d->tempsRecharge = 60.0f;
                d->ventMax = 80;
                break;
        };
    }
    else
        perror("Erreur création Drone\n");
    
	return d;
}

/* Retourne true si le drone reveillé peut/doit livrer le drone, sinon false */
bool canDeliver(int typeColis, int tempsTrajet, int typeDrone)
{
    int i;
    bool deliver = false;
    
    if(typeColis == typeDrone)
    {
        deliver = true;
    }
    else
    {
        if(typeColis < typeDrone)
        {
            deliver = true;
            switch(typeColis)
            {
                case PETIT:
                    
                    if(typeDrone == GROS)
                    {
                        for(i=0; i<NB_DRONE_MOYEN; i++)
                        {
                            Drone drone = vaisseau.dronesM[i];
                            if(tempsTrajet < drone->batterie)
                                deliver = false;
                        }   
                    }
                    
                    for(i=0; i<NB_DRONE_PETIT; i++)
                    {
                        Drone drone = vaisseau.dronesP[i];
                        if(tempsTrajet < drone->batterie)
                            deliver = false;
                    }
                    break;
                case MOYEN:
                    for(i=0; i<NB_DRONE_MOYEN; i++)
                    {
                        Drone drone = vaisseau.dronesM[i];
                        if(tempsTrajet < drone->batterie)
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

/* Met à jour la batterie du Drone en fonction du temps qui s'est écoulé depuis sont dernier appel */
float recharger(float batterie, Type type, time_t *oldTime)
{
    time_t now;
    time(&now);
    double recharge = (double)now -(double)*oldTime;
    *oldTime = now;
    float newBatterie = batterie;
    float maxBatterie;
    
    switch(type)
    {
        case PETIT:
            maxBatterie = 65.0f;
            break;
        case MOYEN:
            maxBatterie = 75.0f;
            break;
        case GROS:
            maxBatterie = 85.0f;
            break;
    }
    
    if((recharge*2 + batterie) >= maxBatterie)
        newBatterie = maxBatterie;
    else
        newBatterie += recharge*2;
    
    return newBatterie;
}

/* Génère la météo */
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
