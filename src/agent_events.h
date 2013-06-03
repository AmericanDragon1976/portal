#ifndef AGENT_EVENTS_H
#define AGENT_EVENTS_H

void monitor_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);
void read_cb(struct bufferevent *buffer_event, void *heads);
void event_cb(struct bufferevent *buffer_event, short what, void *ctx);
void signal_cb(evutil_socket_t signal, short events, void *user_data);
void parse_hook_command(char *text, char *svc, char *command, int len);
int find_service_index(char *name, service *svc_list);
int execute_command(service svc, char *command);

#endif
