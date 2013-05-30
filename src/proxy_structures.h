#ifndef PROXY_STRUCTURES_H
#define PROXY_STRUCTURES_H

service* new_null_svc();
service* new_svc(service *next, struct evconnlistener *listener, struct bufferevent *buffer_event, svc_client_pair *pair);
svc_client_pair* new_null_svc_client_pair();
svc_client_pair* new_svc_client_pair(struct bufferevent *client, struct bufferevent *service);
svc_pack* new_null_svc_package();
svc_client_node* new_null_svc_client_node();
svc_client_node* new_svc_client_node(svc_client_pair *pair, svc_client_node *next);
svc_pack* new_svc_package(service *svc, svc_client_pair *pair);
void free_pair_list(client_list list);

#endif 