#ifndef MONITOR_H
#define MONITOR_H
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


/* creates a new instance of the struct proxy with all pointers inicilized to NULL*/
proxy* newNullProxy ();
/* creates a new instance of the struct proxy with pointers set to passed in value*/
proxy* newProxy (struct bufferevent* bevProxy, proxy* nxt);


#endif 