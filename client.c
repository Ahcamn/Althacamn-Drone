#include "client.h"
    
void *fonc_client(void *arg) 
{
    int clientID = (int)arg + 1;
    // int msgid;
    Client c = createClient();
    
    pthread_mutex_lock(&ville.mutex_client);
    
    if(c->couvert && c->jardin && c->present && c->tempsTrajet <= 30)
    {
       
        pthread_cond_signal (&vaisseau.condition_drone);
        /*if ((msgid = msgget(CLE, 0)) == -1) 
            erreur("Ereur msgget Client\n");
        
        if (msgsnd(msgid, &c, sizeof(Client), 0) == -1) 
            perror("Erreur de l'envoi du message\n");*/

        printf("Client %d  => couvert : %d / jardin : %d / present : %d / tempsTrajet : %d minutes / colis : %d\n", clientID, c->couvert, c->jardin, c->present, c->tempsTrajet, c->colis);
        
    }
        
    pthread_mutex_unlock(&ville.mutex_client);
    
    pthread_exit(NULL);
} 


Client createClient()
{
    Client c = malloc(sizeof(Client));
    
    if(c != NULL)
    {
        c->couvert = pileOuface();
        c->jardin = pileOuface();
        c->present = pileOuface();
        c->tempsTrajet = rand()%21 + 10;
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
    
    if(pthread_join(client, NULL))
            erreur("Erreur pthread_join Client\n");
}

bool pileOuface()
{
    return rand() % 2;
}
