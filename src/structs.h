/* All of the structures used are defined here */

#ifndef STRUCTS_H
#define STRUCTS_H
#include "portal-proxy.h"

/* Keeps referances to the incoming connection from a clinent and the outgoing 
* connection that proxys that client to the correct service. */
typedef struct serviceClientPair {
    struct bufferevent *b_client, *b_service;
    struct serviceClientPair *next;
} servCliPair;

/* Keeps all inportant information about a particular servcie that will be proxy 
* services will be preformed for. This information is stored in a linked list of 
* services. This struct is used for each node in the list. */
typedef struct service {
    char name[30];
    struct service *next;
    char listen[22];    // listen for clients on this address
    char monitor[22];   // connect to monitor programm at the address
    char serv[22];      // address to connect service to
    struct evconnlistener *listener;
    struct bufferevent *b_monitor;
    servCliPair *clientList;
} service;

/* Allows a particular pair of connections - client and the service the client 
* is connected to - and the service they belong to, to be passed as a single 
* argument so that it is not nessary to searc the list of services and their 
* client lists in order to find that match for a partner to the buffer whos event 
* caused a callback to be called. */
typedef struct servicePackage {
    service *serv;
    servCliPair *pair;
} servicePack;


#endif