#include "portal-monitor.h"
#include "monitor.h"
#include "monitorSetup.h"

int 
main (int argc, char **argv) 
{
    moni_serv           *service_list = NULL;
    char                file_name[file_nm_len];
    char                *file_buffer = NULL, **cmd_args = NULL;
    FILE                *file_ptr = NULL;
    int                 file_size;
    struct event_base   *base = NULL;
    struct event        *signal_event = NULL;
    struct event        *to_event;     // TODO: use to query agents on status of service on a timer.

    cmd_args = argv;

    if (!verify_comnd_args(argc, cmd_args)) 
        usage();

    strcpy(file_name, cmd_args[argc - 1]);
    file_size = get_config_file_len(file_name);
    file_buffer = read_file(file_name, file_size);
    service_list = parse_config_file(file_buffer, file_size);
    free(file_buffer);
    
    base = event_base_new();
    contact_agents(base, service_list);
    listen_for_proxys(base, service_list);

        // TODO: call backs for agent event buffer, & body of listen_for_proxys()
        // establish listener(s) for proxy agent(s) 
        // get current address for service(s) being monitored from the agent(s)
        // establish listeners for proxys 
    
    // Monitoring activity 
        // respond to proxy(s) that are requesting inicial data,
        // maintain a list of services, who needs updates on them, and their agents
        // in a cycle, (no more frequently that once every x seconds), request each service's agent test its service and update status
        // broadcast to proxy if address needs changed
}