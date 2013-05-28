/*
 * Proxy provides the proxying services for the portal proxy service. Accepts 
 * with Monitor to know where to direct clients, listens for direction to 
 * change from Monitor and passes traffic back and forth from clients to services.
 */

#include "proxy.h"
#include "proxy_config.h"
#include "proxy_events.h"


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
    printf("    portal-proxy space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    portal-proxy -C ../../deps/config.txt\n");
    exit(0);
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
 * Allocates memory for a new service node and inicilizes all pointer members 
 * to NULL, returns a pointer to this new node.
 */
service* 
new_null_service_node() 
{
    service     *new_node = (service *) calloc(1, sizeof(service));

    new_node->next = NULL;
    new_node->listener = NULL;
    new_node->monitor_buffer_event = NULL;
    new_node->client_list = NULL;

    return (new_node);
}

/* 
 * Allocates memory for a new service node and inicilizes all pointer members 
 * to the values passed in via parameters,  returns a pointer to this new 
 * node. 
 */
service* 
new_service_node(service *nxt, struct evconnlistener *lstnr, 
               struct bufferevent *bevm, svc_client_pair *scp) 
{
    service     *new_node = (service *) calloc(1, sizeof(service));

    new_node->next = nxt;
    new_node->listener = lstnr;
    new_node->monitor_buffer_event = bevm;
    new_node->client_list = scp;

    return new_node;
}

/*  
 * Allocates memeory for a new svc_client_pair and sets their members inicial values to NULL
 */
svc_client_pair* 
new_null_svc_client_pair () {
    svc_client_pair   *new_pair = (svc_client_pair *) malloc(sizeof(svc_client_pair));

    new_pair->client_buffer_event = NULL;
    new_pair->service_buffer_event = NULL;
    new_pair->next = NULL;

    return new_pair;
}


/* 
 * Allocates memeory for a new svc_client_pair and sets their members inicial values as supplied by caller 
 */
svc_client_pair* 
new_svc_client_pair(struct bufferevent *client, struct bufferevent *service, svc_client_pair *nxt)
{
    svc_client_pair   *new_pair = (svc_client_pair *) malloc(sizeof(svc_client_pair));

    new_pair->client_buffer_event = client;
    new_pair->service_buffer_event = service;
    new_pair->next = nxt;

    return new_pair;
}


/* 
 * allocates memory for a new service_package sets all pointers to NULL and returns a pointer to the new
 * service_package 
 */
service_pack* 
new_null_service_package() 
{
    service_pack    *new_svc = (service_pack *) malloc(sizeof(service_pack));

    new_svc->svc = NULL;
    new_svc->pair = NULL;

    return new_svc;
}

/* 
 * allocates memory for a new service_package sets all pointers to the values passed in by parameters and
 * returns a pointer to the new service_package 
 */
service_pack* 
new_svcice_package(service *svc, svc_client_pair *par) 
{
    service_pack    *new_svc = (service_pack *) malloc(sizeof(service_pack));

    new_svc->svc = svc;
    new_svc->pair = par;

    return new_svc;
}

/* 
 * Goes through list of services, and for each service it :
 *   connects to monitor,
 *   requests service addr.
 */
void 
init_services(struct event_base *event_loop, service *service_list) 
{
    service             *svc_list = (service *) service_list;
    struct addrinfo     *server = NULL;
    struct addrinfo     *hints = NULL;

    while (svc_list != NULL) {
        char ip_address[16], port_number[6];
        int i = 0; 
        int j = 0; 

        if (!parse_address(svc_list->monitor, ip_address, port_number))
            fprintf(stderr, "Bad address unable to connect to monitor for %s\n", svc_list->name);
        else {
            hints =  set_criteria_addrinfo();
            i = getaddrinfo(ip_address, port_number, hints, &server);

            if (i != 0){                                                         
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
                svc_list = svc_list->next;
                continue;
            } 

            svc_list->monitor_buffer_event = bufferevent_socket_new(event_loop, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST); 
            bufferevent_setcb(svc_list->monitor_buffer_event, monitor_read_cb, NULL, event_cb, service_list); 

            if(bufferevent_socket_connect(svc_list->monitor_buffer_event, server->ai_addr, server->ai_addrlen) != 0) { 
                fprintf(stderr, "Error connecting to monitor\n"); 
                bufferevent_free(svc_list->monitor_buffer_event); 
            } 

            bufferevent_enable(svc_list->monitor_buffer_event, EV_READ|EV_WRITE);
            bufferevent_write(svc_list->monitor_buffer_event, svc_list->name, sizeof(svc_list->name));
        }
        svc_list = svc_list->next;
    }
}

/* 
 * goes through the list of services, creats a listener to accept new clients
 * for each service in the list 
 */
void 
init_service_listeners(struct event_base *event_loop, service *svc_list) 
{
    int                 port_number_as_int;
    struct sockaddr_in  svc_address;
    struct in_addr      *inp = (struct in_addr *) malloc (sizeof(struct in_addr));

    while (svc_list != NULL) {
        char ip_address[ip_len], port_number[port_len];

        if (!parse_address(svc_list->listen, ip_address, port_number)) {
            fprintf(stderr, "Bad address unable listen for clients for service %s\n", svc_list->name);
        } else {
            port_number_as_int = atoi(port_number);
            inet_aton(ip_address, inp); 
            memset(&svc_address, 0, sizeof(svc_address));
            svc_address.sin_family = AF_INET;
            svc_address.sin_addr.s_addr = (*inp).s_addr; 
            svc_address.sin_port = htons(port_number_as_int); 
            svc_list->listener = evconnlistener_new_bind(event_loop, client_connect_cb, svc_list, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr *) &svc_address, sizeof(svc_address)); 
            if (!svc_list->listener)
                printf("Couldn't create Listener\n");
        }
        svc_list = svc_list->next;
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

/* 
 * Recives a pointer to the linked list of services. For each node in the list
 * frees the monitor buffer, frees the client list, and then frees the node.  
 */
void 
free_all_service_nodes(service *svc_list)
{
    service         *temp = svc_list;

    while (svc_list != NULL) {
        bufferevent_free(svc_list->monitor_buffer_event);
        free_pair_list(svc_list->client_list);
        evconnlistener_free(svc_list->listener);
        svc_list = svc_list->next;
        free(temp);
        temp = svc_list;
    }
}

/*
 * Recives a pointer to a linked list of service client pairs. Frees both 
 * buffer events and then frees the node, for each node in the linked list. 
 */
void 
free_pair_list(svc_client_pair *pair)
{
    svc_client_pair   *temp = pair;

    while (pair != NULL){
        bufferevent_free(pair->client_buffer_event);
        bufferevent_free(pair->service_buffer_event);
        pair = pair->next;
        free(temp);
        temp = pair;
    }
}

int 
main(int argc, char **argv) 
{
    service             *service_list = NULL;
    struct event_base   *event_loop = NULL;
    struct event        *signal_event = NULL;

    /* 
     * Currently -C flag is required but has no effect, if  others are
     * added later to change behavior change the verfyComndlnArgs() function 
     * to change what they do
     */

    if (!validate_args(argc, argv))
        usage();

    service_list = parse_config_file(argv[argc - 1]);    
 
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
    free_all_service_nodes(service_list);
    event_base_free(event_loop);
}
