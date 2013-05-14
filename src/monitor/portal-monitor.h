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


typedef struct monitor_service {
	char name [30];
	char agentAddr[22]; 		// connect to agent on this address
	char proxyAddr[22];			// listen for porxy on this address
	struct evconnlistener *listener;
	struct bufferevent *b_proxy;
	struct bufferevent *b_agent;
	struct monitor_service *next;
} moni_serv;

#endif
