#include "client.h"

void createClientThread(pthread_t client, int i)
{
    if(pthread_create(&client, NULL, fonc_client, (void*)i))
        perror("Erreur création thread Client\n");
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
        c->enAttente = false;
        c->tempsTrajet = rand()%21 + 10;
        c->order = createOrder();
    }
    else
        perror("Erreur création Drone\n");
    
    return c;
}

Order createOrder()
{
    Order order = malloc(sizeof(Order));
    
    if(order != NULL)
    {
        switch(rand()%6)
		{
			case 0:
                order->type = GROS;
                break;
			case 1:
            case 2:
                order->type = MOYEN;
                break;
            default:
                order->type = PETIT;
                break;
		}
       
        order->livre = false;
    }
    else
        perror("Erreur création Commande\n");

    return order;
}

bool alea()
{
    int r = rand() % 100 + 1;
    return r <= 90;
}
