#include "proxy.h"
#include "proxy_config.h"
#include "proxy_events.h"
#include "proxy_structures.h"

/* 
 * Allocates memory for a new service node and inicilizes all pointer members 
 * to NULL, returns a pointer to this new node.
 */
service* 
new_null_service() 
{
    service     *new_svc = (service *) calloc(1, sizeof(service));

    new_svc->listener = NULL;
    new_svc->monitor_buffer_event = NULL;
    new_svc->client_list = NULL;

    return (new_svc);
}

/* 
 * Allocates memory for a new service node and inicilizes all pointer members 
 * to the values passed in via parameters,  returns a pointer to this new 
 * node. 
 */
service* 
new_service(struct evconnlistener *listener, struct bufferevent *moniter_buffer_event, svc_client_pair *pair) 
{
    service     *new_node = (service *) calloc(1, sizeof(service));

    new_node->listener = listener;
    new_node->monitor_buffer_event = moniter_buffer_event;
    new_node->client_list = pair;

    return new_node;
}

/*  
 * Allocates memeory for a new svc_client_pair and sets their members inicial values to NULL
 */
svc_client_pair* 
new_null_svc_client_pair () 
{
    svc_client_pair   *new_pair = (svc_client_pair *) malloc(sizeof(svc_client_pair));

    new_pair->client_buffer_event = NULL;
    new_pair->service_buffer_event = NULL;
    new_pair->next = NULL;

    return (new_pair);
}


/* 
 * Allocates memeory for a new svc_client_pair and sets their members inicial values as supplied by caller 
 */
svc_client_pair* 
new_svc_client_pair(struct bufferevent *client, struct bufferevent *service, svc_client_pair *next)
{
    svc_client_pair   *new_pair = (svc_client_pair *) malloc(sizeof(svc_client_pair));

    new_pair->client_buffer_event = client;
    new_pair->service_buffer_event = service;
    new_pair->next = next;

    return (new_pair);
}


/* 
 * Allocates memory for a new service_package sets all pointers to NULL and returns a pointer to the new
 * service_package 
 */
service_pack* 
new_null_service_package() 
{
    service_pack    *new_svc_pack = (service_pack *) malloc(sizeof(service_pack));

    new_svc_pack->svc = NULL;
    new_svc_pack->pair = NULL;

    return (new_svc_pack);
}

/* 
 * allocates memory for a new service_package sets all pointers to the values passed in by parameters and
 * returns a pointer to the new service_package 
 */
service_pack* 
new_service_package(service *svc, svc_client_pair *pair) 
{
    service_pack    *new_svc_pack = (service_pack *) malloc(sizeof(service_pack));

    new_svc_pack->svc = svc;
    new_svc_pack->pair = pair;

    return (new_svc_pack);
}

/*
 * Recives a pointer to a linked list of service client pairs. Frees both 
 * buffer events and then frees the node, for each node in the linked list. 
 */
void 
free_pair_list(svc_client_pair *pair)
{
    svc_client_pair   *temp = pair;

    while (pair != NULL){
        bufferevent_free(pair->client_buffer_event);
        bufferevent_free(pair->service_buffer_event);
        pair = pair->next;
        free(temp);
        temp = pair;
    }
}

