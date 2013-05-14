#ifndef MONITORSETUP_H
#define MONITORSETUP_H

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

bool 
verify_comnd_args(int argc, char **argv);

int 
get_config_file_len(char *name);

char* 
read_file(char *name, int len);

moni_serv* 
parse_config_file(char *buff, int len);

moni_serv* 
new_null_moni_serv_node();

moni_serv* 
new_moni_serv_node(struct evconnlistener* lstnr, moni_serv* service, struct bufferevent *bevProxy, struct bufferevent *bevAgent);

bool 
parse_address(char *addr_to_parse, char *ip_addr, char* port_num);

void 
contact_agents(struct event_base *base, moni_serv *s_list);

void 
listen_for_proxys(struct event_base *base, moni_serv *s_list);

struct addrinfo* 
set_criteria_addrinfo();

#endif 