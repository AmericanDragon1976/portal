/* Handles the callbacks and memory management needed to actually proxy clients. */

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

/* 
 * Call back for information comming in from the monitor, in the buffer info. Function 
 * should recive a c-string that contains the name of the service and the ip address and 
 * port number in the format a.b.c.d:port_num.
 * Call back will verify is the right service, and store the address including port num
 * in the serv member of the service struct, and then if the address is new will terminate
 * all clients connected and free memory. 
 */
void 
monitor_read_cb(struct bufferevent *bev, void *serv);

/* 
 * when triggered by a connecting client determines which service the client was connecting
 * for and connects them to the appropiate service. 
 */
void 
client_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
    struct sockaddr *address, int socklen, void *ctx);

/* 
 * Call back for information comming in from either a client or the service they are connected
 * to, function passes the info through. If the service is not there will attempt to 
 * reconnect and send the info. 
 */
void 
proxy_read_cb(struct bufferevent *bev, void *srv_pck);

/* 
 * triggered by all event buffer event, reports errors and successful connects. 
 */
void 
event_cb(struct bufferevent *bev, short what, void *ctx);

/* 
 * Catches interrupt signals and allows the program to cleanup before exiting. 
 */
void 
signal_cb(evutil_socket_t sig, short events, void *user_data);

/*  
 * Allocates memeory for a new serv_cli_pair and sets their pointer members 
 * inicial values to NULL.
 */
serv_cli_pair* 
new_null_serv_cli_pair();

/* 
 * allocates memeory for a new serv_cli_pair and sets their members inicial 
 * values as supplied by caller 
 */
serv_cli_pair* 
new_serv_cli_pair(struct bufferevent *client, struct bufferevent *service, 
    serv_cli_pair *nxt);

/* 
 * allocates memory for a new service_package sets all pointers to NULL and 
 * returns a pointer to the new service_package 
 */
service_pack* 
new_null_service_package();

/* 
 * allocates memory for a new service_package sets all pointers to the values 
 * passed in by parameters and returns a pointer to the new service_package 
 */
service_pack* 
new_service_package(service *srvs, serv_cli_pair *par);

/* 
 * goes through the list of services, frees the listeners associated with that
 * service 
 */
void 
free_all_listeners (service *serv_list);

/* 
 * goes through the list of services, frees memory allocated for each node 
 */
void 
free_all_service_nodes (service *serv_list);

#endif