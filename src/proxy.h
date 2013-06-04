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

extern int list_size, i;

void usage ();
void initalize_array(service service_list[]);
bool validate_args(int argc, char **argv);
void init_services (struct event_base *event_loop, service *svc_list);
void init_service_listeners(struct event_base *event_loop, service *svc_list);
struct addrinfo* set_criteria_addrinfo();
void init_signals(struct event_base *event_loop);

#endif