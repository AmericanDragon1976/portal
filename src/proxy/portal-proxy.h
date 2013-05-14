/* Runs the event loop. the corrensponding .c file is the function main.*/

#ifndef PORTALPROXY_H
#define PORTALPROXY_H
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

/* 
 * Keeps referances to the incoming connection from a clinent and the outgoing 
 * connection that proxys that client to the correct service. 
 */
typedef struct service_client_pair {
    struct bufferevent *b_client, *b_service;
    struct service_client_pair *next;
} serv_cli_pair;

/* 
 * Keeps all inportant information about a particular servcie that will be proxy 
 * services will be preformed for. This information is stored in a linked list of 
 * services. This struct is used for each node in the list. 
 */
typedef struct service {
    char name[30];
    struct service *next;
    char listen[22];    // listen for clients on this address
    char monitor[22];   // connect to monitor programm at the address
    char serv[22];      // address to connect service to
    struct evconnlistener *listener;
    struct bufferevent *b_monitor;
    serv_cli_pair *client_list;
} service;

/* 
 * Allows a particular pair of connections - client and the service the client 
 * is connected to - and the service they belong to, to be passed as a single 
 * argument so that it is not nessary to searc the list of services and their 
 * client lists in order to find that match for a partner to the buffer whos event 
 * caused a callback to be called. 
 */
typedef struct service_package {
    service *serv;
    serv_cli_pair *pair;
} service_pack;

#endif