#include "monitor.h"
#include "monitor_config.h"
#include "monitor_events.h"

/*
 * 
 */
void 
proxy_connect_callback(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx)
{}

/*
 * 
 */
void 
signal_cb(evutil_socket_t signal, short events, void *user_data)
{}