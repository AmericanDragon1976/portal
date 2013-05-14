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

void 
monitor_read_cb(struct bufferevent *bev, void *serv);

void 
client_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
    struct sockaddr *address, int socklen, void *ctx);

void 
proxy_read_cb(struct bufferevent *bev, void *srv_pck);

void 
event_cb(struct bufferevent *bev, short what, void *ctx);

void 
signal_cb(evutil_socket_t sig, short events, void *user_data);

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
free_all_listeners (service *serv_list);

void 
free_all_service_nodes (service *serv_list);

#endif