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

/*                   PRTOTYPES                 */

/* 
 * Verifys the command line areguament for the monitor, returns ture if correct 
 * argumets are supplied and false otherwise. 
 */
bool 
verify_comnd_args(int argc, char **argv);

/* 
 * returns the length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array once it is read in. 
 */
int 
get_config_file_len(char *name);

/* 
 * Read a file, Recive name of file with length of <= 100, and int var (len) by referance
 * return pointer to c string (buffer) holding contents of the file, int will now contain
 * length of the of the buffer. len needs set so that the info is avaliable later. 
 */
char* 
read_file(char *name, int len);

/* 
 * recives a service pointer to buffer containing config file
 * returns pointer that is the head of a list of services. 
 */
moni_serv* 
parse_config_file(char *buff, int len);

/* 
 * creates a new instance of a moni_serv struct with all pointers inicilized to NULL 
 */
moni_serv* 
new_null_moni_serv_node();

/* 
 * creates a new instance of moni_serve struct with pointers set to the passed in values 
 */
moni_serv* 
new_moni_serv_node(struct evconnlistener* lstnr, moni_serv* service, struct bufferevent *bevProxy, struct bufferevent *bevAgent);

/* 
 * Takes an adrress in the form a.b.c.d:port_number and parses it storing the ip
 * address and port number in the approiate char arrays, addr_to_parse[22], ip_addr[16] 
 * and port_num[6], returns true if successful otherwise returns false 
 */
bool 
parse_address(char *addr_to_parse, char *ip_addr, char* port_num);

/* 
 * Connects to the agent for each service 
 */
void 
contact_agents(struct event_base *base, moni_serv *s_list);

/* 
 * creates a listener for each servics to listen for the proxys and establish the buffer events for them at that time 
 */
void 
listen_for_proxys(struct event_base *base, moni_serv *s_list);

/*
 * sets the information in in and addrinfo structure to be used as the critera for stuctrue passed into getaddrinfo() 
 */
struct addrinfo* 
set_criteria_addrinfo();

#endif 