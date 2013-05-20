#include "portal-agent.h" 
#include "agentSetup.h"
#include "agent.h"

void print_cmd_lst(serv_lst *list)
{
    while (list != NULL){ 
        hook_path_pair  *temp = list->cmd_lst;
        while (temp != NULL){ 
            printf("command: %s, Path: %s\n", temp->hook, temp->path);
            temp = temp->next;
        }
        list = list->next;
    }
}

/*
 * main
 */
int
main (int argc, char **argv) 
{
    char                    file_name[file_nm_len];
    char                    *file_buffer = NULL, **cmd_args = NULL;
    FILE                    *file_ptr = NULL;
    list_heads              ser_n_bevs;
    int                     file_size = 0;
    struct event_base       *base = NULL;
    struct event            *signal_event = NULL;
    struct evconnlistener   *listener = NULL;

    ser_n_bevs.s_list = NULL;
    ser_n_bevs.b_list = NULL;

    cmd_args = argv;

    if (!verify_comnd_ln_args(argc, cmd_args)) 
    	usage();

    strcpy(file_name, cmd_args[argc - 1]);
    file_size = get_config_file_len(file_name);
    file_buffer = read_file(file_name, file_size);
    ser_n_bevs.s_list = parse_config_file(file_buffer, file_size); 
    free(file_buffer);
    
    base = event_base_new();
    listen_for_monitors(base, listener, &ser_n_bevs);

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *) base);
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }

    event_base_dispatch(base);
//    evconnlistener_free(listener);
    free_lists_memory(&ser_n_bevs);
    event_base_free(base);
}