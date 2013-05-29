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

#define svc_name_len            30      // length of all char arrays holding service names
#define complete_address_len    22      // length of char arrays holding address a.d.c.e:portnum
#define file_name_len           100     // length of file names
#define ip_len                  16      // length of ip portion of address
#define port_len                6       // length of port number portion of address
#define number_services         100     // number of services that can be handled at the same time with out reallocating for a larger array. 

extern int list_size;

void usage ();
bool validate_args(int argc, char **argv);
void init_services (struct event_base *event_loop, service *svc_list);
void init_service_listeners(struct event_base *event_loop, service *svc_list);
struct addrinfo* (set_criteria_addrinfo);
void init_signals(struct event_base *event_loop);

#endif