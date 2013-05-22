/*
 * Proxy provides the proxying services for the portal proxy service. Accepts 
 * with Monitor to know where to direct clients, listens for direction to 
 * change from Monitor and passes traffic back and forth from clients to services.
 */

#include "proxy.h"
#include "proxy_config.h"
#include "proxy_events.h"


/* 
 * called by to_ event every time it times out. They body is commented out
 * because its current use is for testing some aspects and it may be removed 
 * from the final product. 
 */
void 
timeout_cb(evutil_socket_t fd, short what, void *arg) 
{ 
/*  service *test = (service *) arg;
    bufferevent_write(test->b_monitor, test->name, sizeof(test->name));
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
    service     *nw_node = (service *) calloc(1, sizeof(service));

    nw_node->next = NULL;
    nw_node->listener = NULL;
    nw_node->b_monitor = NULL;
    nw_node->client_list = NULL;

    return (nw_node);
}

/* 
 * Allocates memory for a new service node and inicilizes all pointer members 
 * to the values passed in via parameters,  returns a pointer to this new 
 * node. 
 */
service* 
new_service_node(service *nxt, struct evconnlistener *lstnr, 
               struct bufferevent *bevm, serv_cli_pair *scp) 
{
    service     *nw_node = (service *) calloc(1, sizeof(service));

    nw_node->next = nxt;
    nw_node->listener = lstnr;
    nw_node->b_monitor = bevm;
    nw_node->client_list = scp;

    return nw_node;
}

/*  
 * allocates memeory for a new serv_cli_pair and sets their members inicial values to NULL
 */
serv_cli_pair* 
new_null_serv_cli_pair () {
    serv_cli_pair   *nw_pair = (serv_cli_pair *) malloc(sizeof(serv_cli_pair));

    nw_pair->b_client = NULL;
    nw_pair->b_service = NULL;
    nw_pair->next = NULL;

    return nw_pair;
}


/* 
 * allocates memeory for a new serv_cli_pair and sets their members inicial values as supplied by caller 
 */
serv_cli_pair* 
new_serv_cli_pair(struct bufferevent *client, struct bufferevent *service, serv_cli_pair *nxt)
{
    serv_cli_pair   *nw_pair = (serv_cli_pair *) malloc(sizeof(serv_cli_pair));

    nw_pair->b_client = client;
    nw_pair->b_service = service;
    nw_pair->next = nxt;

    return nw_pair;
}


/* 
 * allocates memory for a new service_package sets all pointers to NULL and returns a pointer to the new
 * service_package 
 */
service_pack* 
new_null_service_package() 
{
    service_pack    *nw_serv = (service_pack *) malloc(sizeof(service_pack));

    nw_serv->serv = NULL;
    nw_serv->pair = NULL;

    return nw_serv;
}

/* 
 * allocates memory for a new service_package sets all pointers to the values passed in by parameters and
 * returns a pointer to the new service_package 
 */
service_pack* 
new_service_package(service *srvs, serv_cli_pair *par) 
{
    service_pack    *nw_serv = (service_pack *) malloc(sizeof(service_pack));

    nw_serv->serv = srvs;
    nw_serv->pair = par;

    return nw_serv;
}

/* 
 * Goes through list of services, and for each service it :
 *   connects to monitor,
 *   requests service addr.
 */
void 
init_services(struct event_base *eBase, service *service_list) 
{
    service             *serv_list = (service *) service_list;
    struct addrinfo     *server = NULL;
    struct addrinfo     *hints = NULL;

    while (serv_list != NULL) {
        char ip_addr[16], port_num[6];
        int i = 0; 
        int j = 0; 

        if (!parse_address(serv_list->monitor, ip_addr, port_num))
            fprintf(stderr, "Bad address unable to connect to monitor for %s\n", serv_list->name);
        else {
            hints =  set_criteria_addrinfo();
            i = getaddrinfo(ip_addr, port_num, hints, &server);

            if (i != 0){                                                         
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
                serv_list = serv_list->next;
                continue;
            } 

            serv_list->b_monitor = bufferevent_socket_new(eBase, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST); 
            bufferevent_setcb(serv_list->b_monitor, monitor_read_cb, NULL, event_cb, service_list); 

            if(bufferevent_socket_connect(serv_list->b_monitor, server->ai_addr, server->ai_addrlen) != 0) { 
                fprintf(stderr, "Error connecting to monitor\n"); 
                bufferevent_free(serv_list->b_monitor); 
            } 

            bufferevent_enable(serv_list->b_monitor, EV_READ|EV_WRITE);
            bufferevent_write(serv_list->b_monitor, serv_list->name, sizeof(serv_list->name));
        }
        serv_list = serv_list->next;
    }
}

/* 
 * goes through the list of services, creats a listener to accept new clients
 * for each service in the list 
 */
void 
init_service_listeners(struct event_base *eBase, service *serv_list) 
{
    int                 port_no;
    struct sockaddr_in  serv_addr;
    struct in_addr      *inp = (struct in_addr *) malloc (sizeof(struct in_addr));

    while (serv_list != NULL) {
        char ip_addr[ip_len], port_num[port_len];

        if (!parse_address(serv_list->listen, ip_addr, port_num)) {
            fprintf(stderr, "Bad address unable listen for clients for service %s\n", serv_list->name);
        } else {
            port_no = atoi(port_num);
            inet_aton(ip_addr, inp); 
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = (*inp).s_addr; 
            serv_addr.sin_port = htons(port_no); 
            serv_list->listener = evconnlistener_new_bind(eBase, client_connect_cb, serv_list, 
                                                          LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, 
                                                         -1, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
            if (!serv_list->listener)
                printf("Couldn't create Listener\n");
        }
        serv_list = serv_list->next;
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
free_all_service_nodes(service *serv_list)
{
    service         *temp = serv_list;

    while (serv_list != NULL) {
        bufferevent_free(serv_list->b_monitor);
        free_pair_list(serv_list->client_list);
        evconnlistener_free(serv_list->listener);
        serv_list = serv_list->next;
        free(temp);
        temp = serv_list;
    }
}

/*
 * Recives a pointer to a linked list of service client pairs. Frees both 
 * buffer events and then frees the node, for each node in the linked list. 
 */
void 
free_pair_list(serv_cli_pair *pair)
{
    serv_cli_pair   *temp = pair;

    while (pair != NULL){
        bufferevent_free(pair->b_client);
        bufferevent_free(pair->b_service);
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

    // This time out event only needed for testing, can be removed in final version
 /*   struct timeval      five_seconds = {5, 0};
    struct event        *to_event; 

    to_event = event_new(event_loop, -1, EV_PERSIST, timeout_cb, service_list);
    event_add(to_event, &five_seconds);
*/
    kill_event = evsignal_new(event_loop, SIGINT, signal_cb, (void *) event_loop);
    if (!kill_event || event_add(kill_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }

    event_base_dispatch(event_loop);
    free_all_service_nodes(service_list);
    event_base_free(event_loop);
}
