#ifndef PROXY_H
#define PROXY_H
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

#define svc_name_len            30      // length of all char arrays holding service names
#define complete_address_len    22      // length of char arrays holding address a.d.c.e:portnum
#define file_name_len           100     // length of file names
#define ip_len                  16      // length of ip portion of address
#define port_len                6       // length of port number portion of address
#define number_services         100     // number of services that can be handled at the same time with out reallocating for a larger array. 

extern int list_size;

/* 
 * Keeps referances to the incoming connection from a clinent and the outgoing 
 * connection that proxys that client to the correct service. 
 */
typedef struct svc_client_pair {
    struct bufferevent          *client_buffer_event, *svc_buffer_event;
} svc_client_pair;

/*
 * Each node points to the next node in the list, and has a pointer to the instance of the structure holding the acutal data.
 */
typedef struct svc_client_node{
    svc_client_pair         	*pair;
    struct svc_client_node    	*next;
}svc_client_node;	

/*
 *This structure is a list of clients, head points to the first node in the list.
 */
typedef struct client_list {
    svc_client_node     *head;
} client_list;

/* 
 * Keeps all inportant information about a particular servcie that will be proxy 
 * services will be preformed for. This information is stored in a linked list of 
 * services. This struct is used for each node in the list. 
 */
typedef struct service {
    char                    name[svc_name_len];
    char                    listen[complete_address_len];    // listen for clients on this address
    char                    monitor[complete_address_len];   // connect to monitor programm at the address
    char                    svc[complete_address_len];       // address to connect service to
    struct evconnlistener   *listener;
    struct bufferevent      *monitor_buffer_event;
    client_list             list_of_clients;
} service;

/* 
 * Allows a particular pair of connections - client and the service the client 
 * is connected to - and the service they belong to, to be passed as a single 
 * argument so that it is not nessary to search the list of services and their 
 * client lists in order to find that match for a partner to the buffer whos event 
 * caused a callback to be called. 
 */
typedef struct svc_package {
    service           *svc;
    svc_client_pair   *pair;
} svc_pack;

void usage ();
bool validate_args(int argc, char **argv);
void init_services (struct event_base *event_loop, service *svc_list);
void init_service_listeners(struct event_base *event_loop, service *svc_list);
struct addrinfo* set_criteria_addrinfo();
void init_signals(struct event_base *event_loop);

#endif