#include "client.h"

/* Crée les threads Client */
void createClientThread(pthread_t *client, int nbClients)
{
    int i;
    
    for(i=0; i < nbClients; i++)
    {
        if(pthread_create(&client[i], NULL, fonc_client, (void*)(intptr_t)i))
            perror("Erreur création thread Client\n");
        
        usleep(100*IN_MILLISECONDS);
    }   
}

/* Crée et initialise la structure d'un Client */   
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

/* Crée et initialise la structure d'un Colis */
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

/* Retourne un nombre aléatoire entre 1 et 100 */
bool alea()
{
    int r = rand() % 100 + 1;
    return r <= 90;
}
