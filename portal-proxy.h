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

typedef struct serviceClientPair {
    struct bufferevent *b_client, *b_service;
    struct serviceClientPair *next;
} servCliPair;

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

typedef struct servicePackage {
    service *serv;
    servCliPair *pair;
} servicePack;

static void usage();
static char* readFile(char *name, long *len);
static service* parseConfigFile(char *buffer, long fileSize);
static void initServices(struct event_base *eBase, service *servList);
static void initServiceListeners(struct event_base *eBase, service *servList);
static void onClientConnect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);
void freeAllListeners(service *servList);
void freeAllServiceNodes(service *servList);
static void signal_cb(evutil_socket_t sig, short events, void *user_data);
static void monitorRead (struct bufferevent *bev, void *serv);
static void proxyRead(struct bufferevent *bev, void *srvPck);
static void cbEvent(struct bufferevent *bev, short what, void *ctx);