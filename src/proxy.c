/*
 * Proxy provides the proxying services for the portal proxy service. Accepts 
 * with Monitor to know where to direct clients, listens for direction to 
 * change from Monitor and passes traffic back and forth from clients to services.
 */

#include "proxy_structures.h"
#include "proxy.h"
#include "proxy_config.h"
#include "proxy_events.h"

int list_size;

/* 
 * Called by to_ event every time it times out. They body is commented out
 * because its current use is for testing some aspects and it may be removed 
 * from the final product. 
 */
void 
timeout_cb(evutil_socket_t fd, short what, void *arg) 
{ 
/*  service *test = (service *) arg;
    bufferevent_write(test->monitor_buffer_event, test->name, sizeof(test->name));
    printf("timeout_cb called\n");
    printf("Server up\n");
*/
}

/* 
 * Prints to screen the proper syntax for running the program, then exits.
 */
void 
usage()
{
    printf("Usage is as follows: \n");
    printf("    proxy space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    proxy -C ../../deps/config.txt\n");
    exit(0);
}

void
initalize_array(service service_list[])
{
    int i = 0;

    for (i = 0; i < list_size; i++){
        strcpy(service_list[i].name, "none");
        strcpy(service_list[i].listen, "0.0.0.0:0000");
        strcpy(service_list[i].monitor, "0.0.0.0:0000");
        strcpy(service_list[i].svc, "0.0.0.0:0000");
        service_list[i].listener = NULL;
        service_list[i].monitor_buffer_event = NULL;
        service_list[i].list_of_clients->head = NULL;
    }
}

/* 
 * Verifies the command line arguments. The arguments must include flag -C and 
 * the path/to/comfig.txt. If specific uses of the -C flag or other flags are 
 * added then this function should be altered accordingly.
 */
bool 
validate_args(int argc, char **argv) 
{

    if (argc < 3)         
        return (false);

    if (strcmp(argv[1], "-C") != 0) // if other flags added or effect of -C flag changes alter here. 
        return (false);

    return (true);
}

/* 
 * Goes through list of services, and for each service it :
 *   connects to monitor,
 *   requests service addr.
 */
void 
init_services(struct event_base *event_loop, service svc_list[]) 
{
    int                 current_svc = 0;
    struct addrinfo     *server = NULL;
    struct addrinfo     *hints = NULL;

    while (current_svc < list_size && strcmp(svc_list[current_svc].name, "none") != 0) {
        char ip_address[ip_len], port_number[port_len];
        int i = 0; 
        int j = 0; 

        if (!parse_address(svc_list[current_svc].monitor, ip_address, port_number))
            fprintf(stderr, "Bad address unable to connect to monitor for %s\n", svc_list->name);
        else {
            hints =  set_criteria_addrinfo();
            i = getaddrinfo(ip_address, port_number, hints, &server);

            if (i != 0){                                                         
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
                current_svc++;
                continue;
            } 

            svc_list[current_svc].monitor_buffer_event = bufferevent_socket_new(event_loop, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST); 
            bufferevent_setcb(svc_list[current_svc].monitor_buffer_event, monitor_read_cb, NULL, event_cb, svc_list); 

            if(bufferevent_socket_connect(svc_list[current_svc].monitor_buffer_event, server->ai_addr, server->ai_addrlen) != 0) { 
                fprintf(stderr, "Error connecting to monitor\n"); 
                bufferevent_free(svc_list->monitor_buffer_event); 
            } 

            bufferevent_enable(svc_list[current_svc].monitor_buffer_event, EV_READ|EV_WRITE);
            bufferevent_write(svc_list[current_svc].monitor_buffer_event, svc_list[current_svc].name, sizeof(svc_list[current_svc].name));
        }
        current_svc++;
    }
}

/* 
 * goes through the list of services, creats a listener to accept new clients
 * for each service in the list 
 */
void 
init_service_listeners(struct event_base *event_loop, service svc_list[]) 
{
    int                 port_number_as_int;
    int                 current_svc = 0;
    struct sockaddr_in  svc_address;
    struct in_addr      *ip_bytes = (struct in_addr *) malloc (sizeof(struct in_addr));

    while (current_svc < list_size && strcmp(svc_list[current_svc].name, "none") != 0) {
        char ip_address[ip_len], port_number[port_len];

        if (!parse_address(svc_list[current_svc].listen, ip_address, port_number)) {
            fprintf(stderr, "Bad address unable listen for clients for service %s\n", svc_list[current_svc].name);
        } else {
            port_number_as_int = atoi(port_number);
            inet_aton(ip_address, ip_bytes); 
            memset(&svc_address, 0, sizeof(svc_address));
            svc_address.sin_family = AF_INET;
            svc_address.sin_addr.s_addr = (*ip_bytes).s_addr; 
            svc_address.sin_port = htons(port_number_as_int); 
            svc_list[current_svc].listener = evconnlistener_new_bind(event_loop, client_connect_cb, &svc_list[current_svc], LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr *) &svc_address, sizeof(svc_address)); 
            if (!svc_list[current_svc].listener)
                printf("Couldn't create Listener\n");
        }
        current_svc++;
    }
}

/*
 * Recoved the event_loop (event base) and adds an event to it that will trigger 
 * when a signal is recived. It handles those signals, for the kill signal it frees
 * all memory and shuts down the event loop instead of simply letting it crash.
 */
void init_signals(struct event_base *event_loop)
{    
    struct event *signal_event = NULL;

    signal_event = evsignal_new(event_loop, SIGINT, signal_cb, (void *) event_loop);
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }
}

/* 
 * Sets the information in an addrinfo structure to be used as the critera 
 * stuctrue passed into getaddrinfo().
 */
struct addrinfo* 
set_criteria_addrinfo() 
{
    struct addrinfo     *hints = (struct addrinfo *) malloc(sizeof(struct addrinfo));

    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = 0;
    hints->ai_protocol = 0;

    return hints;
}

int 
main(int argc, char **argv) 
{
    list_size = number_services;
    service             service_list[list_size];
    struct event_base   *event_loop = NULL;
    struct event        *signal_event = NULL;

    /* 
     * Currently -C flag is required but has no effect, if  others are
     * added later to change behavior change the verfyComndlnArgs() function 
     * to change what they do
     */

    if (!validate_args(argc, argv))
        usage();

    initalize_array(service_list);
    parse_config_file(argv[argc - 1], service_list);    
 
    event_loop = event_base_new();

    init_services(event_loop, service_list);
    init_service_listeners(event_loop, service_list); 

/* 
    This time out event only needed for testing, can be removed in final version
    struct timeval      five_seconds = {5, 0};
    struct event        *to_event; 

    to_event = event_new(event_loop, -1, EV_PERSIST, timeout_cb, service_list);
    event_add(to_event, &five_seconds);
*/
    init_signals(event_loop);
    event_base_dispatch(event_loop);
    event_base_free(event_loop);
}
