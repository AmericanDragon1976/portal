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

/* 
 * Keeps referances to the incoming connection from a clinent and the outgoing 
 * connection that proxys that client to the correct service. 
 */
typedef struct service_client_pair {
    struct bufferevent          *client_buffer_event, *service_buffer_event;
    struct service_client_pair  *next;
} svc_client_pair;

/* 
 * Keeps all inportant information about a particular servcie that will be proxy 
 * services will be preformed for. This information is stored in a linked list of 
 * services. This struct is used for each node in the list. 
 */
typedef struct service {
    char                    name[svc_name_len];
    struct                  service *next;
    char                    listen[complete_address_len];    // listen for clients on this address
    char                    monitor[complete_address_len];   // connect to monitor programm at the address
    char                    svc[complete_address_len];      // address to connect service to
    struct evconnlistener   *listener;
    struct bufferevent      *monitor_buffer_event;
    svc_client_pair         *client_list;
} service;

/* 
 * Allows a particular pair of connections - client and the service the client 
 * is connected to - and the service they belong to, to be passed as a single 
 * argument so that it is not nessary to searc the list of services and their 
 * client lists in order to find that match for a partner to the buffer whos event 
 * caused a callback to be called. 
 */
typedef struct service_package {
    service           *svc;
    svc_client_pair   *pair;
} service_pack;

void usage ();
bool validate_args(int argc, char **argv);
service* new_null_service_node();
service* new_service_node(service *next, struct evconnlistener *listener, struct bufferevent *buffer_event, svc_client_pair *scp);
svc_client_pair* new_null_svc_client_pair();
svc_client_pair* new_svc_client_pair(struct bufferevent *client, struct bufferevent *service, svc_client_pair *next);
service_pack* new_null_service_package();
service_pack* new_svcice_package(service *svc, svc_client_pair *pair);
void init_services (struct event_base *event_loop, service *svc_list);
void init_service_listeners(struct event_base *event_loop, service *svc_list);
struct addrinfo* set_criteria_addrinfo();
void init_signals(struct event_base *event_loop);
void free_all_service_nodes (service *svc_list);
void free_pair_list(svc_client_pair *pair);

#endif