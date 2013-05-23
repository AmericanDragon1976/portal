#ifndef AGENT_EVENTS_H
#define AGENT_EVENTS_H

void 
monitor_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
                   struct sockaddr *address, int socklen, void *ctx);

void
read_cb(struct bufferevent *bev, void *heads);

void
event_cb(struct bufferevent *bev, short what, void *ctx);

void 
signal_cb(evutil_socket_t sig, short events, void *user_data);

void 
parse_hook_command(char *text, char *serv, char *cmd, int len);

svc_lst*
find_service(char* serv, svc_lst *services);

int
execute_command(svc_lst *service, char *cmd);
#endif