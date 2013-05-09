#ifndef PORTALMONITOR_H
#define PORTALMONITOR_H
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

/*                STRUCTS                 */

typedef struct proxy {
	struct bufferevent *bProxy;
	struct proxy *next;
} proxy;

typedef struct monitorService {
	char name [30];
	char agentAddr[22];
	char proxyAddr[22];
	struct evconnlistener *listener;
	proxy *proxyList;
	struct monitorService *next;
} moniServ;

/*                   PRTOTYPES                 */
#endif
