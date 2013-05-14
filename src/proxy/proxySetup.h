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

/* 
 * Verifies the command line arguments. The arguments must include flag -C and 
 * the path/to/comfig.txt. If specific uses of the -C flag or other flags are 
 * added then this function should be altered accordingly.
 */
bool 
verify_comnd_ln_args(int argc, char **argv);

/* 
 * Prints to screen the proper syntax for running the program, then exits.
 */
void 
usage ();

/* 
 * Recives the name and path to athe config file. Returns an int congtining the
 * length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array after it is read in. 
 */
int 
get_config_file_len(char *name);

/* 
 * Recives the name a path to the config file, and a int containing the length 
 * of the file. Returns pointer to a char array holding contents of the file.  
 */
char* 
read_file(char *name, int len);

/* 
 * Allocates memory for a new service node and inicilizes all pointer members 
 * to NULL, returns a pointer to this new node.
 */
service* 
new_null_service_node();

/* 
 * Allocates memory for a new service node and inicilizes all pointer members 
 * to the values passed in via parameters,  returns a pointer to this new 
 * node. 
 */
service* 
new_service_node(service *nxt, struct evconnlistener *lstnr, 
    struct bufferevent *bevm, serv_cli_pair *scp);

/* 
 * Recives a char pointer to the buffer containing the config file text. 
 * Returns a pointer to the head of a linked list of services. 
 */
service* 
parse_config_file (char *buffer, int file_size);

/* 
 * goes through list of services and for each service:
 *   connects to monitor
 *   requests service addr 
 */
void 
init_services (struct event_base *eBase, service *serv_list);

/* 
 * Takes an adrress in the form a.b.c.d:port_number and parses it storing the ip
 * address and port number in the approiate char arrays, addrToParse[22], ip_addr[16] 
 * and port_num[6], returns true if successful otherwise returns false 
 */
bool 
parse_address(char *addrToParse, char *ip_addr, char* port_num);

/* 
 * goes through the list of services, creats a listener to accept new clients
 * for each service in the list 
 */
void 
init_service_listeners (struct event_base *eBase, service *serv_list);

/* 
 * Sets the information in an addrinfo structure to be used as the critera 
 * stuctrue passed into getaddrinfo().
 */
struct 
addrinfo* set_criteria_addrinfo ();

#endif