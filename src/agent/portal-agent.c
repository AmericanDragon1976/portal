#include "portal-agent.h" 
#include "agentSetup.h"

/*
 * main
 */
int
main (int argc, char **argv) 
{
    char                file_name[file_nm_len];
    char                *file_buffer = NULL, **cmd_args = NULL;
    FILE                *file_ptr = NULL;
    int                 file_size;
    struct event_base   *base = NULL;
    struct event        *signal_event = NULL;

    cmd_args = argv;

    if (!verify_comnd_args(argc, cmd_args)) 
    	usage();

    strcpy(file_name, cmd_args[argc - 1]);
    file_size = get_config_file_len(file_name);
    file_buffer = read_file(file_name, file_size);
    service_list = parse_config_file(file_buffer, file_size);
    free(file_buffer);
    
	// TODO: endlessly listen for updates from the monitor(s)
    base = event_base_new();
    listen_for_monitors(base, service_list);

	// TODO: execute hooks as told by the monitor. 
}