/* Handles the callbacks and memory management needed to actually proxy clients. */

#ifndef "PROXY_H"
#define "PROXY_H"
#include "portal-proxy.h"
#include "structs.h"

/* Call back for information comming in from the monitor, in the buffer info. Function 
* should recive a c-string that contains the name of the service and the ip address and 
* port number in the format a.b.c.d:portnum.
* Call back will verify is the right service, and store the address including port num
* in the serv member of the service struct, and then if the address is new will terminate
* all clients connected and free memory. */
static void monitorReadCB (struct bufferevent *bev, void *serv);

/* when triggered by a connecting client determines which service the client was connecting
* for and connects them to the appropiate service. */
static void clientConnectCB (struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);

/* Call back for information comming in from either a client or the service they are connected
* to, function passes the info through. If the service is not there will attempt to 
* reconnect and send the info. */
static void proxyReadCB (struct bufferevent *bev, void *srvPck);

/* triggered by all event buffer event, reports errors and successful connects. */
sstatic void eventCB (struct bufferevent *bev, short what, void *ctx);

/* catches interrupt signal and allows the program to cleanup before exiting. */
static void signalCB (evutil_socket_t sig, short events, void *user_data);

/* goes through the list of services, frees the listeners associated with that
* service */
void freeAllListeners (service *servList);

/* goes through the list of services, frees memory allocated for each node */
void freeAllServiceNodes (service *servList);

#endif