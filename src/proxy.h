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

#define serv_nm_siz     30      // length of all char arrays holding service names
#define comp_add_len    22      // length of char arrays holding address a.d.c.e:portnum
#define file_nm_len     100     // length of file names
#define ip_len          16      // length of ip portion of address
#define port_len        6       // length of port number portion of address

/* 
 * Keeps referances to the incoming connection from a clinent and the outgoing 
 * connection that proxys that client to the correct service. 
 */
typedef struct service_client_pair {
    struct bufferevent          *b_client, *b_service;
    struct service_client_pair  *next;
} serv_cli_pair;

/* 
 * Keeps all inportant information about a particular servcie that will be proxy 
 * services will be preformed for. This information is stored in a linked list of 
 * services. This struct is used for each node in the list. 
 */
typedef struct service {
    char                    name[serv_nm_siz];
    struct                  service *next;
    char                    listen[comp_add_len];    // listen for clients on this address
    char                    monitor[comp_add_len];   // connect to monitor programm at the address
    char                    serv[comp_add_len];      // address to connect service to
    struct evconnlistener   *listener;
    struct bufferevent      *b_monitor;
    serv_cli_pair           *client_list;
} service;

/* 
 * Allows a particular pair of connections - client and the service the client 
 * is connected to - and the service they belong to, to be passed as a single 
 * argument so that it is not nessary to searc the list of services and their 
 * client lists in order to find that match for a partner to the buffer whos event 
 * caused a callback to be called. 
 */
typedef struct service_package {
    service         *serv;
    serv_cli_pair   *pair;
} service_pack;

void 
usage ();

bool 
validate_args(int argc, char **argv);

service* 
new_null_service_node();

service* 
new_service_node(service *nxt, struct evconnlistener *lstnr, 
    struct bufferevent *bevm, serv_cli_pair *scp);

serv_cli_pair* 
new_null_serv_cli_pair();

serv_cli_pair* 
new_serv_cli_pair(struct bufferevent *client, struct bufferevent *service, 
    serv_cli_pair *nxt);

service_pack* 
new_null_service_package();

service_pack* 
new_service_package(service *srvs, serv_cli_pair *par);

void 
init_services (struct event_base *eBase, service *serv_list);

void 
init_service_listeners(struct event_base *eBase, service *serv_list);

struct 
addrinfo* set_criteria_addrinfo();

void 
free_all_service_nodes (service *serv_list);

void 
free_pair_list(serv_cli_pair *pair);
#endif