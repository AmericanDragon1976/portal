#include "agent_structures.h"
#include "agent.h" 
#include "agent_config.h"
#include "agent_events.h"

/*
 * Allocates memory for a new service, sets all pointer members to NULL, and
 * returns a pointer to it. 
 */
service*
new_null_svc()
{
    service        *new_svc = (service *) malloc(sizeof(service));

    new_svc_list->list_of_hooks = NULL;

    return (new_svc);
}

/*
 * Allocates memory for a new service, sets all pointer members to the value 
 * passed in and returns a pointer to it. 
 */
service*
new_svc(hook_path_pair *hook_list_head)
{
    service        *new_svc = (service *) malloc(sizeof(service));

    new_svc->list_of_hooks = hook_list_head;

    return (new_svc);
}

/*
 * Allocates memory for a new hook_path_node, sets all pointers to NULL and returns a pointer to the node.
 */
hook_path_node* 
new_null_hook_path_node()
{
    hook_path_node      *new_node = (hook_path_node *) malloc(sizeof(hook_path_node));

    new_node->pair = NULL;
    new_node->next = NULL;

    return (new_node);
}

/*
 * Allocates memory for a new hook_path_node, set all pointer to the values passed in and and returns a pointer to the node.
 */
hook_path_node* 
new_hook_path_node(hook_path_pair *pair, hook_path_node *nxt)
{
    hook_path_node      *new_node = (hook_path_node *) malloc(sizeof(hook_path_node));

    new_node->pair = pair;
    new_node->next = nxt;

    return (new_node);
}

/*
 * Allocates memory for a new hook list, sets the pointer to NULL and returns a pointer to the list. 
 */
hook_list* 
new_null_hook_list()
{
    hook_list       *new_list = (hook_list *) malloc(sizeof(hook_list));

    new_list->head = NULL;

    return (new_list);
}

/*
 * Allocates memory for a new hook list, set the pointer to the value that was passed in and returns a pointer to the list. 
 */
hook_list* 
new_hook_list(hook_path_node *head)
{
    hook_list       *new_list = (hook_list *) malloc(sizeof(hook_list));

    new_list->head = head;

    return (new_list);
}

/*
 * Allocates memory for a new buffer list node, sets all the pointers to NULL and returns a pointer to the node. 
 */
buffer_list_node* 
new_null_buffer_list()
{
    buffer_list_node        *new_node = (buffer_list_node *) malloc (sizeof(buffer_list_node));

    new_node->next = NULL;
    new_node->bev = NULL;

    return (new_node);
}

/*
 * Allocates memory for a new buffer list node, sets all the pointers to the values passed in and returns a pointer to the node.
 */
buffer_list_node* 
new_buffer_list(buffer_list_node *nxt, struct bufferevent *bev)
{
    buffer_list_node        *new_node = (buffer_list_node *) malloc (sizeof(buffer_list_node));

    new_node->next = nxt;
    new_node->bev = bev;

    return (new_node);
}

/*
 * Recives a pointer to a buffer list with each node holding a pointer to a bufferevent. Frees
 * each bufferevent and the node holding it. 
 */
void 
free_buffers(buffer_list *list_of_buffers)
{
    buffer_list_node     *current_node = list_of_buffers->head;

    while (list_of_buffers->head != NULL){
        bufferevent_free(current_node->bev);
        list_of_buffers->head = current_node->next;
        free(current_node);
        current_node = list_of_buffers->head;
    }
}


/*
 * Recives a pointer to a list of commands and frees all memory associated
 * with that list. 
 */
void 
free_hook_list(hook_list *hooks)
{
    hook_path_node  *current_node = hooks->head;

    while (hooks->head != NULL){
        hooks->head = current_node->next;
        free(current_node);
        current_node = hooks->head;
    }
}
