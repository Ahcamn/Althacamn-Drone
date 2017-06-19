#include "client.h"

    
Client createClient(int i)
{
    Client c = malloc(sizeof(Client));
    
    if(c != NULL)
    {
        c->clientID = i;
        c->couvert = alea();
        c->jardin = alea();
        c->present = alea();
        c->tempsTrajet = rand()%21 + 10;
        // c->tempsTrajet = 30;
        c->colis = rand()%3 + 1;
    }
    else
        erreur("Erreur création Drone\n");
    
    return c;
}

void createClientThread(pthread_t client, int i)
{
    if(pthread_create(&client, NULL, fonc_client, (void*)i))
        erreur("Erreur création thread Client\n");
}

bool alea()
{
    int r = rand() % 100 + 1;
    return r <= 90;
}
