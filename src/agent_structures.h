#ifndef AGENT_STRUCTURES_H
#define AGENT_STRUCTURES_H
#define svc_name_len             30      // length of all char arrays holding service names
#define complete_address_len     22      // length of char arrays holding address a.d.c.e:portnum
#define file_name_len            100     // length of file names including path to them
#define ip_len                   16      // length of ip portion of address
#define port_len                 6       // length of port number portion of address
#define hook_len                 40      // largest length allowed for a hook name from agent config file
#define monitor_address          "127.0.0.1:4000"
#define reply_len                500     // largest size for a reply from an executed file. 
#define no_such_command          404     // the int returned when attempting to execute a hook/command and there is no match to be used.

/*
 * Structure to hold a hook and the associated exetutable file path including file name. 
 */
typedef struct hook_path_pair {
    char                    hook[hook_len];
    char                    path[file_name_len];
} hook_path_pair;

/*
 * Structure to make a linked list of hook path pairs. Includes a pointer to the next node and a pointer to a structure holding the data. 
 */
typedef struct hook_path_node {
    struct hook_path_node   *next;
    hook_path_pair 			*pair;
} hook_path_node;

/*
 * A list of Hooks. The pointer points to the head node.
 */
 typedef struct hook_list{
 	hook_path_node 	 		*head;
 } hook_list;

/*
 * Structure to hold a service with a pointer to it list of hooks. 
 */
typedef struct service {
    char            	name[svc_name_len];
    hook_list 			*list_of_hooks;
} service;

/*
 * Structure to make a node in a linked list of bufferevents. Holds a pointer to the next node and a pointer to the structure holding the buffer event. 
 */
typedef struct buffer_list_node {
    struct buffer_list_node     *next;
    struct bufferevent 	      	*bev;
} buffer_list_node;

/*
 * Structure to keep track of the head node of the linked list of bufferevents.
 */
typedef struct buffer_list {
    buffer_list *list_of_buffer_events;
} buffer_list;

svc_list* new_null_svc();
svc_list* new_svc(hook_path_pair *hook_list_head);
hook_path_node* new_null_hook_path_node();
hook_path_node* new_hook_path_node(hook_path_pair *pair, hook_path_node *nxt);
hook_list* new_null_hook_list();
hook_list* new_hook_list(hook_path_node *head);
buffer_list_node* new_null_buffer_list();
buffer_list_node* new_buffer_list(buffer_list_node *nxt, struct bufferevent *bev);
void free_buffers(buffer_list *bevs);
void free_hook_list(hook_list *hooks);

#endif 