#ifndef AGENT_H
#define AGENT_H

void 
monitor_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
                   struct sockaddr *address, int socklen, void *ctx);

void
read_cb(struct bufferevent *bev, void *heads);

void
event_cb(struct bufferevent *bev, short what, void *ctx);
#endif