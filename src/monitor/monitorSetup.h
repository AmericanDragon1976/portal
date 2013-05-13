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

/* Verifys the command line areguament for the monitor, returns ture if correct 
* argumets are supplied and false otherwise. */
bool verifyComndArgs(int argc, char **argv);

/* returns the length of the config file. There is a good probility that 
* the file will contain 0's and so strlen() can not be used to determine 
* the length of the char array once it is read in. */
int getConfigFileLen(char *name);

/*  Read a file, Recive name of file with length of <= 100, and int var (len) by referance
* return pointer to c string (buffer) holding contents of the file, int will now contain
* length of the of the buffer. len needs set so that the info is avaliable later. */
char* readFile(char *name, int len);

/* recives a service pointer to buffer containing config file
* returns pointer that is the head of a list of services. */
moniServ* parseConfigFile(char *buff, int len);

/* creates a new instance of a moniServ struct with all pointers inicilized to NULL */
moniServ* newNullMoniServNode();
/* creates a new instance of moniServe struct with pointers set to the passed in values */
moniServ* newMoniServNode(struct evconnlistener* lstnr, proxy* prxyLst, moniServ* service);
#endif 