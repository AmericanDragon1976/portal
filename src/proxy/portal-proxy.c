#include "portal-proxy.h"
#include "proxySetup.h"
#include "proxy.h"

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

int 
main(int argc, char **argv) 
{
    int                 i = 0;
    service             *service_list = NULL;
    char                file_name[100];
    FILE                *filePointer = NULL;
    int                 file_size;
    char                *file_buffer = NULL, **cmd_args = NULL;
    struct event_base   *base = NULL;
    struct event        *signal_event = NULL;

    /* 
     * Currently -C flag is required but has no effect, if  others are
     * added later to change behavior change the verfyComndlnArgs() function 
     * to change what they do
     */

    cmd_args = argv;
    if (!verify_comnd_ln_args(argc, cmd_args))
        usage();

    strcpy(file_name, argv[2]);
    file_size = get_config_file_len(file_name);
    file_buffer = read_file(file_name, file_size);
    service_list = parse_config_file(file_buffer, file_size);    
    free(file_buffer);
 
    base = event_base_new();

    init_services(base, service_list);
    init_service_listeners(base, service_list); 

    // This time out event only needed for testing, can be removed in final version
    struct timeval      five_seconds = {5, 0};
    struct event        *to_event; 

    to_event = event_new(base, -1, EV_PERSIST, timeout_cb, service_list);
    event_add(to_event, &five_seconds);

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *) base);
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }

    event_base_dispatch(base);
    free_all_listeners(service_list);
    free_all_service_nodes(service_list);
    event_base_free(base);
}
