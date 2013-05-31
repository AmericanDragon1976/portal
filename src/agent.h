#ifndef AGENT_H
#define AGENT_H

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

extern int list_size;

void usage(); 
bool validate_args(int argc, char **argv);
bool parse_address(char *address_to_parse, char *ip_address, char* port_number);
void listen_for_monitors(struct event_base *base, struct evconnlistener *listner, list_heads *heads);
void init_signals(struct event_base *event_loop);

#endif
