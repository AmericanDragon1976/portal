/* 
 * Containd all the functions used to setup the portal proxy. Verifys that 
 * all needed informatin is included from the command line, flags and the config 
 * file's name and location. Parses the config file, creates the list of services 
 * and requests the starting address to proxy clients to for each service. In 
 * general anything need to prepare to recive client connections. 
 */

#ifndef SETUP_H
#define SETUP_H

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
verify_comnd_ln_args(int argc, char **argv);

void 
usage ();

int 
get_config_file_len(char *name);

char* 
read_file(char *name, int len);

service* 
new_null_service_node();

service* 
new_service_node(service *nxt, struct evconnlistener *lstnr, 
    struct bufferevent *bevm, serv_cli_pair *scp);

service* 
parse_config_file (char *buffer, int file_size);

void 
init_services (struct event_base *eBase, service *serv_list);

bool 
parse_address(char *addrToParse, char *ip_addr, char* port_num);

void 
init_service_listeners(struct event_base *eBase, service *serv_list);

struct 
addrinfo* set_criteria_addrinfo();

#endif