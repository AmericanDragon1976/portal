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

#define svc_name_len             30      // length of all char arrays holding service names
#define complete_address_len     22      // length of char arrays holding address a.d.c.e:portnum
#define file_name_len            100     // length of file names including path to them
#define ip_len                   16      // length of ip portion of address
#define port_len                 6       // length of port number portion of address
#define hook_len                 40      // largest length allowed for a hook name from agent config file
#define monitor_address          "127.0.0.1:4000"
#define reply_len                500     // largest size for a reply from an executed file. 
#define no_such_command          404     // the int returned when attempting to execute a hook/command and there is no match to be used.

typedef struct hook_path_pair {
    struct hook_path_pair   *next;
    char                    hook[hook_len];
    char                    path[file_name_len];
} hook_path_pair;

typedef struct service_list {
    struct service_list         *next;
    char                        name[svc_name_len];
    hook_path_pair              *hook_list;
} svc_list;

typedef struct buffer_list {
    struct buffer_list      *next;
    struct bufferevent      *bev;
} buffer_list;

typedef struct {
    svc_list    *list_of_services;
    buffer_list *list_of_buffer_events;
} list_heads;

void usage(); 
bool validate_args(int argc, char **argv);
svc_list* new_null_svc_list();
svc_list* new_svc_list(svc_list *nxt, hook_path_pair *hook_list_head);
hook_path_pair* new_null_hook_path_pair();
hook_path_pair* new_hook_path_pair(hook_path_pair *nxt);
bool parse_address(char *address_to_parse, char *ip_address, char* port_number);
void listen_for_monitors(struct event_base *base, struct evconnlistener *listner, list_heads *heads);
void init_signals(struct event_base event_loop);
void free_lists_memory(list_heads *heads);
void free_service_nodes(svc_list *service_list);
void free_command_list(hook_path_pair *commands);
void free_buffers(buffer_list *bevs);

#endif
