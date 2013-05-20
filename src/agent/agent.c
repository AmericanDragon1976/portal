#include "portal-agent.h" 
#include "agentSetup.h"
#include "agent.h"

/*
 * Called when a monitor connects. Sets up an event buffer for that monitor and
 * adds it to the event base. 
 */
void 
monitor_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
                   struct sockaddr *address, int socklen, void *ctx)
{
    list_heads          *lists = (list_heads *) ctx;
    buffer_list         *temp_buff_list = (buffer_list *) malloc(sizeof(buffer_list));
    struct event_base   *base = evconnlistener_get_base(listener);
    struct bufferevent  *temp_bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);

    bufferevent_setcb(temp_buff_list->bev, read_cb, NULL, event_cb, lists);
    bufferevent_enable(temp_buff_list->bev, EV_READ|EV_WRITE);
    temp_buff_list->next = lists->b_list;
    lists->b_list = temp_buff_list;
}

/*
 * Called when the bufferevent connected to a monitor is ready to be read.
 * Recives the calling event and a pointer to a struct that holds a pointer
 * to the linked lists of services and a pointer to the linked list of 
 * bufferevents. Reads what command is recived from monitor and executes that
 * file associated with that command for that service, and then sends the 
 * results back to monitor. 
 */
void
read_cb(struct bufferevent *bev, void *heads)
{
    struct      evbuffer *input = bufferevent_get_input(bev);
    char        *text, *serv, *cmd;
    serv_lst    *temp_service;
    int         len, result;
    list_heads  *lst_hds = (list_heads *) heads;

    len = evbuffer_get_length(input);
    text = (char *) malloc(len);
    bzero(text, len); 
    evbuffer_remove(input, text, len); 

    parse_hook_command(text, serv, cmd, len);
    temp_service = find_service(serv, lst_hds->s_list);
    result = execute_command(temp_service, cmd);
    bufferevent_write(bev, &result, reply_len);
}

/*
 * Recives the text which includes the service for the command to be preformed 
 * on and the hook or command to be preformed. 
 * Parses the text and stores the service name in serv, and the hook or command
 * in cmd. 
 */
void
parse_hook_command(char *text, char *serv, char* cmd, int len)
{
    int i, j, k;

    for (k = 0; text[k++] != ' '; ) 
        ;

    serv = (char *) malloc(k);

    for (i = 0; i > k; i++)
        serv[i] = text[i];

    text[i - 1] = '\0';
    cmd = (char *) malloc(len - k + 1);

    for ( j = 0; i > len; i++)
        cmd[j++] = text[i];

    cmd[i] = '\0';
}

/*
 * Recive the name of the service, steps throught the linked list of services
 * and returns a pointer to the named service's node. If there is no match
 * returns NULL. 
 */
serv_lst*
find_service(char *serv, serv_lst *services)
{
    serv_lst *curr_serv = services;

    while (curr_serv != NULL) {
        if (strcmp(curr_serv->name, serv) != 0)
            curr_serv = curr_serv->next;
        else
            return (curr_serv);
    }

    return (curr_serv);
}

/*
 * Recives a pointer to the service node for the named service and what the hook
 * or command is. Uses the cmd to find the file to be executed from the list of 
 * commands for that service, executes it and returns any responce. 
 */
 int
 execute_command(serv_lst *service, char *cmd)
 {
    hook_path_pair      *hook_lst = service->cmd_lst;

    while (hook_lst != NULL && strcmp(hook_lst->hook, cmd) != 0)
        hook_lst = hook_lst->next;

    if (hook_lst != NULL)
        return (system(hook_lst->path));
    else 
        return (no_such_command);
 }

/*
 * Called when there is an event on a bufferevent. Recives the calling event,
 * what the event was, and a pointer to a struct which has pointers to the 
 * linked list of services and the linked list of bufferevents. 
 */
void
event_cb(struct bufferevent *bev, short what, void *ctx)
{ 
    if (what & BEV_EVENT_ERROR) {
        unsigned long err;

        while ((err = (bufferevent_get_openssl_error(bev)))) { printf("1\n");
            const char *msg = (const char*)
                ERR_reason_error_string(err);
            const char *lib = (const char*)
                ERR_lib_error_string(err);
            const char *func = (const char*)
                ERR_func_error_string(err);
            fprintf(stderr,
                "%s in %s %s\n", msg, lib, func);
        }

        if (errno)
            perror("connection error");
    }

    if (what & BEV_EVENT_EOF)
        printf("EOF\n");

    if (what & BEV_EVENT_CONNECTED)
        printf("CONNECTION SUCCESSFUL\n");

    if (what & BEV_EVENT_TIMEOUT)
        printf("TIMEOUT\n");
}

/*
 * Recives the struct with a pointer to the two lists, the buffer list, and the
 * service list. Calls the functions to realease the memory for each of the lists.
 */
void
free_lists_memory(list_heads *heads)
{
    free_buffers(heads->b_list); 
    free_service_nodes(heads->s_list); 
}

/*
 * Recives a pointer to the linked list of service nodes and frees all memory 
 * associated with that list. 
 */
void 
free_service_nodes(serv_lst *service_list)
{
    serv_lst    *temp = service_list;

    while (service_list != NULL){ 
        free_cmd_lst(service_list->cmd_lst);
        service_list = service_list->next;
        free(temp);
        temp = service_list;
    }
}

/*
 * Recives a pointer to a linked list of commands and frees all memory associated
 * with that list. 
 */
void 
free_cmd_lst(hook_path_pair *cmds)
{
    hook_path_pair  *temp = cmds;

    while (cmds != NULL){
        cmds = cmds->next;
        free(temp);
        temp = cmds;
    }
}

/*
 * Recives a pointer to a linked list with each node holding a bufferevent. Frees
 * each bufferevent and the node holding it. 
 */
void 
free_buffers(buffer_list *bevs)
{
    buffer_list     *temp = bevs;

    while (bevs != NULL){
        bufferevent_free(bevs->bev);
        bevs = bevs->next;
        free(temp);
        temp = bevs;
    }
}

/* 
 * catches interrupt signal and allows the program to cleanup before exiting. 
 */
void 
signal_cb (evutil_socket_t sig, short events, void *user_data) 
{
    struct event_base   *base = (struct event_base *) user_data;
    struct timeval      delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");


    event_base_loopexit(base, &delay);
}