#ifndef PORTALAGENT_H
#define PORTALAGENT_H

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

#define serv_nm_siz     30      // length of all char arrays holding service names
#define comp_add_len    22      // length of char arrays holding address a.d.c.e:portnum
#define file_nm_len     100     // length of file names
#define ip_len          16      // length of ip portion of address
#define port_len        6       // length of port number portion of address
#define monitor_addr	"127.0.0.1:4000"

typedef struct command_path_pair {
	struct command_path_pair	*next;
	char 						*hook;
	char 						*path;
} hook_path_pair;

typedef struct service_list {
	struct service_list 		*next;
	hook_path_pair				*cmd_lst;
} serv_lst;

typedef struct list_of_buffer_events {
	struct list_or_buffer_events 	*next;
	struct bufferevent 				*bev;
} buffer_list;

typedef struct {
	serv_list 	*s_list;
	buffer_list *b_list;
} list_heads;
#endif
