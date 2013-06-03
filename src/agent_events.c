#include "agent_structures.h"
#include "agent.h" 
#include "agent_config.h"
#include "agent_events.h"

/*
 * Called when a monitor connects. Sets up an event buffer for that monitor and
 * adds it to the event base. 
 */
void  
monitor_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx)
{
    service             *svc_list = (service *) ctx;
    struct event_base   *base = evconnlistener_get_base(listener);
    struct bufferevent  *temp_buffer_event = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
    buffer_list_node    *temp_node = (buffer_list_node *) malloc(sizeof(buffer_list_node));

    bufferevent_setcb(temp_buffer_event, read_cb, NULL, event_cb, svc_list);
    bufferevent_enable(temp_buffer_event, EV_READ|EV_WRITE);
    temp_node->bev = temp_buffer_event;
    temp_node->next = buffer_event_list.head;                     // buffer_event_list is a global variable
    buffer_event_list.head = temp_node;
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
read_cb(struct bufferevent *buffer_event, void *svc_list)
{
    struct evbuffer     *input = bufferevent_get_input(buffer_event);
    char                *text, *svc, *command;
    int                 len, result, svc_location_index;
    service             *array_of_svc = (service *) svc_list;

    len = evbuffer_get_length(input);
    text = (char *) malloc(len);
    bzero(text, len); 
    evbuffer_remove(input, text, len); 

    parse_hook_command(text, svc, command, len);
    svc_location_index = find_service_index(svc, svc_list);
    result = execute_command(array_of_svc[svc_location_index], command);
    if (result > -1)
        bufferevent_write(buffer_event, &result, sizeof(int));
    else
        fprintf(stderr, "ERROR: command recived for a service not in the list of services. ");
}

/*
 * Recives the text which includes the service for the command to be preformed 
 * on and the hook or command to be preformed. 
 * Parses the text and stores the service name in svc, and the hook or command
 * in command. 
 */
void
parse_hook_command(char *text, char *svc, char* command, int len)
{
    int i, j, k;

    for (k = 0; text[k++] != ' '; ) 
        ;

    svc = (char *) malloc(k);

    for (i = 0; i > k; i++)
        svc[i] = text[i];

    text[i - 1] = '\0';
    command = (char *) malloc(len - k + 1);

    for ( j = 0; i > len; i++)
        command[j++] = text[i];

    command[i] = '\0';
}

/*
 * Recive the name of the service, steps throught the array of services
 * and returns a pointer to the named service. If there is no match
 * returns NULL. 
 */
int 
find_service_index(char *name, service *svc_list)
{
    int             current_svc = 0;

    while (current_svc < list_size) {
        if (strcmp(svc_list[current_svc].name, name) != 0)
            current_svc++;
        else
            return (current_svc);
    }

    return (-1);
}

/*
 * Recives a pointer to the service node for the named service and what the hook
 * or command is. Uses the command to find the file to be executed from the list of 
 * commands for that service, executes it and returns any responce. 
 */
 int 
 execute_command(service svc, char *command)
 {
    hook_path_node      *hook_list = svc.list_of_hooks->head;

    while (hook_list != NULL && strcmp(hook_list->pair->hook, command) != 0)
        hook_list = hook_list->next;

    if (hook_list != NULL)
        return (system(hook_list->pair->path));
    else 
        return (no_such_command);
 }

/*
 * Called when there is an event on a bufferevent. Recives the calling event,
 * what the event was, and a pointer to a struct which has pointers to the 
 * linked list of services and the linked list of bufferevents. 
 */
void 
event_cb(struct bufferevent *buffer_event, short what, void *ctx)
{ 
    if (what & BEV_EVENT_ERROR) {
        unsigned long err;

        while ((err = (bufferevent_get_openssl_error(buffer_event)))) { printf("1\n");
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