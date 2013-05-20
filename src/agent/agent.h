#ifndef AGENT_H
#define AGENT_H

void 
monitor_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
                   struct sockaddr *address, int socklen, void *ctx);

void
read_cb(struct bufferevent *bev, void *heads);

void
event_cb(struct bufferevent *bev, short what, void *ctx);

void 
parse_hook_command(char *text, char *serv, char *cmd, int len);

serv_lst*
find_service(char* serv, serv_lst *services);

int
execute_command(serv_lst *service, char *cmd);

void
free_lists_memory(list_heads *heads);

void 
free_service_nodes(serv_lst *service_list);

void 
free_cmd_lst(hook_path_pair *cmds);

void 
free_buffers(buffer_list *bevs);

void 
signal_cb(evutil_socket_t sig, short events, void *user_data);
#endif	