#include "client.h"

void createClientThread(pthread_t client, int i)
{
    if(pthread_create(&client, NULL, fonc_client, (void*)i))
        erreur("Erreur création thread Client\n");
}
    
Client createClient(int i)
{
    Client c = malloc(sizeof(Client));
    
    if(c != NULL)
    {
        c->clientID = i;
        c->couvert = alea();
        c->jardin = alea();
        c->present = alea();
        c->satisfait = true;
        c->tempsTrajet = rand()%21 + 10;
        c->order = createOrder();
    }
    else
        erreur("Erreur création Drone\n");
    
    return c;
}

Order createOrder()
{
    Order order = malloc(sizeof(Order));
    
    if(order != NULL)
    {
        order->type = rand()%3 + 1;;
        order->livre = false;
    }
    else
        erreur("Erreur création Commande\n");

    return order;
}

bool alea()
{
    int r = rand() % 100 + 1;
    return r <= 90;
}
