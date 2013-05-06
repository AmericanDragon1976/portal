/* Containd all the functions used to setup the portal proxy. Verifys that 
* all needed informatin is included from the command line, flags and the config 
* file's name and location. Parses the config file, creates the list of services 
* and requests the starting address to proxy clients to for each service. In 
* general anything need to prepare to recive client connections. */

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

/* verifies command line arguments, must have flag -C and path/to/comfig.txt if 
* specific uses of the -C flag or other flags are added this function should be 
* altered accordingly */
bool verifyComndLnArgs(int argc, char **argv);

/* prints to screen proper syntax for running the program, then exits */
void usage ();

/* returns the length of the config file. There is a good probility that 
* the file will contain 0's and so strlen() can not be used to determine 
* the length of the char array once it is read in. */
int getConfigFileLen(char *name);

/*  Read a file, Recive name of file with length of 100, and long var (len) by referance
* return pointer to c string (buffer) holding contents of the file, int will now contain
* length of the of the buffer. len needs set so that the info is avaliable later. */
char* readFile (char *name, int len);

/* recives a service pointer to buffer containing config file
* returns pointer that is the head of a list of services. */
service* parseConfigFile (char *buffer, int fileSize);

/* goes through list of services and for each service:
*   connects to monitor
*   requests service addr */
void initServices (struct event_base *eBase, service *servList);

/* Takes an adrress in the form a.b.c.d:portnumber and parses it storing the ip
* address and port number in the approiate char arrays, addrToParse[22], ipAddr[16] 
* and portNum[6], returns true if successful otherwise returns false */
bool parseAddress(char *addrToParse, char *ipAddr, char* portNum);

/* goes through the list of services, creats a listener to accept new clients
* for each service in the list */
void initServiceListeners (struct event_base *eBase, service *servList);

#endif