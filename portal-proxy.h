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

typedef struct __ServiceClientPair {
	struct bufferevent *b_client, *b_service;
	struct __ServiceClientPair *next;
} ServCliPair;

typedef struct __Service {
	char name[30];
    struct __Service *next;
    char listen[22];	// listen for clients on this address
    char monitor[22];	// connect to monitor programm at the address
    char serv[22];		// address to connect service to
    struct evconnlistener *listener;
	struct bufferevent *b_monitor;
	ServCliPair *clientList;
	// buffer events for communication with client, communication with service these two should be linked and contained in a stuct
	// that creats a linked list of clients for that service, and a third for the monitor to listen for updates to service status. 

} Service;

typedef struct __ServicePackage {
	Service *serv;
	ServCliPair *pair;
} ServicePack;

static void usage();
static char* readFile(char *name, long *len);
static Service* parseConfigFile(char *buffer, long fileSize);
static void initServices(struct event_base *eBase, Service *servList);
static void initServiceListeners(struct event_base *eBase, Service *servList);
static void onClientConnect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);
void freeAllListeners(Service *servList);
void freeAllServiceNodes(Service *servList);
static void signal_cb(evutil_socket_t sig, short events, void *user_data);
static void monitorRead (struct bufferevent *bev, void *serv);
static void proxyRead(struct bufferevent *bev, void *srvPck);
static void cbEvent(struct bufferevent *bev, short what, void *ctx);