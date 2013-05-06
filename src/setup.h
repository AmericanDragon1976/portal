/* Containd all the functions used to setup the portal proxy. Verifys that 
* all needed informatin is included from the command line, flags and the config 
* file's name and location. Parses the config file, creates the list of services 
* and requests the starting address to proxy clients to for each service. In 
* general anything need to prepare to recive client connections. */

#ifndef SETUP_H
#define SETUP_H
#include "portal-proxy.h"
#include "structs.h"

/* prints to screen proper syntax for running the program, then exits */
static void usage ();

/*  Read a file, Recive name of file with length of 100, and long var (len) by referance
* return pointer to c string (buffer) holding contents of the file, int will now contain
* length of the of the buffer. len needs set so that the info is avaliable later. */
static char* readFile (char *name, long *len);

/* recives a service pointer to buffer containing config file
* returns pointer that is the head of a list of services. */
static service* parseConfigFile (char *buffer, long fileSize);

/* goes through list of services and for each service:
*   connects to monitor
*   requests service addr */
static void initServices (struct event_base *eBase, service *servList);

/* goes through the list of services, creats a listener to accept new clients
* for each service in the list */
static void initServiceListeners (struct event_base *eBase, service *servList);

#endif