#ifndef PROXY_STRUCTURES_H
#define PROXY_STRUCTURES_H

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
    svc_client_pair         *pair;
    svc_client_node         *next;
}svc_client_node;

/*
 *This structure is a list of clients, head points to the first node in the list.
 */
typedef struct client_list {
    svc_client_node     *head;
} client_list;
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

service* new_null_svc();
service* new_svc(service *next, struct evconnlistener *listener, struct bufferevent *buffer_event, svc_client_pair *pair);
svc_client_pair* new_null_svc_client_pair();
svc_client_pair* new_svc_client_pair(struct bufferevent *client, struct bufferevent *service);
svc_pack* new_null_svc_package();
svc_client_node* new_null_svc_client_node();
svc_client_node* new_svc_client_node(svc_client_pair *pair, svc_client_node *next);
svc_pack* new_svc_package(service *svc, svc_client_pair *pair);
void free_pair_list(svc_client_pair *pair);

#endif 