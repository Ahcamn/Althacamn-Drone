#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <stdbool.h>

#include "vaisseau.h"

typedef struct ClientStruct
{
    bool couvert;
    bool jardin;
    bool present;
    float distance;
    int meteo;
}ClientStruct;

typedef ClientStruct* Client;

#endif