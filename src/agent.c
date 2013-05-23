#include "agent.h" 
#include "agent_config.h"
#include "agent_events.h"

/*
 * Outputs to the user how to start the program. 
 */
void 
usage()
{
    printf("Usage is as follows: \n");
    printf("    agent space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    ./agent -C ../../deps/config.txt\n");
    exit(0);
}

/* 
 * Verifys the command line areguament for the monitor, returns ture if correct 
 * argumets are supplied and false otherwise. 
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
 * Allocates memory for a new svc_lst, sets all pointer members to NULL, and
 * returns a pointer to it. 
 */
svc_lst*
new_null_svc_lst()
{
    svc_lst        *nw_svc_lst = (svc_lst *) malloc(sizeof(svc_lst));

    nw_svc_lst->next = NULL;
    nw_svc_lst->cmd_lst = NULL;

    return (nw_svc_lst);
}

/*
 * Allocates memory for a new svc_lst, sets all pointer members to the value 
 * passed in and returns a pointer to it. 
 */
svc_lst*
new_svc_lst(svc_lst *nxt, hook_path_pair *cmd_lst_head)
{
    svc_lst        *nw_svc_lst = (svc_lst *) malloc(sizeof(svc_lst));

    nw_svc_lst->next = nxt;
    nw_svc_lst->cmd_lst = cmd_lst_head;

    return (nw_svc_lst);
}

/*
 * Allocates memory for a new hook_path_pair, sets all pointer members to NULL
 * and returns a pointer to it.
 */
hook_path_pair*
new_null_hook_path_pair()
{
    hook_path_pair     *nw_hok_pth_par = (hook_path_pair *) malloc(sizeof(hook_path_pair));

    nw_hok_pth_par->next = NULL;

    return (nw_hok_pth_par);
}

/*
 * Allocates memory for a new hook_path_pair, sets all pointer members to the
 * value passed in and returns a pointer to it. 
 */
hook_path_pair*
new_hook_path_pair(hook_path_pair *nxt)
{
    hook_path_pair     *nw_hok_pth_par = (hook_path_pair *) malloc(sizeof(hook_path_pair));

    nw_hok_pth_par->next = nxt;

    return (nw_hok_pth_par);
}

/* 
 * Takes an adrress in the form a.b.c.d:port_number and parses it storing the ip
 * address and port number in the approiate char arrays, addrToParse[22], ip_addr[16] 
 * and port_num[6], returns true if successful otherwise returns false 
 */
bool 
parse_address(char *addrToParse, char *ip_addr, char* port_num) 
{
    int     i, j;
    bool    port_now = false;

    j = 0;

    if ( addrToParse == NULL)
        return port_now;

    for (i = 0; i < comp_add_len; ){
        if (addrToParse[i] == ':') {
            i++;
            ip_addr[j] = '\0';
            port_now = true; 
            j = 0;
        }

        if (port_now == false)
            ip_addr[j++] = addrToParse[i++];
        else 
            port_num[j++] = addrToParse[i++];
    }

    port_num[j] = '\0';

    return port_now;
}

/*
 * Open listeners on a preset ip address port combination to recieve monitors
 * connections and instructions. Recives the event base to add events to, the 
 * listener create, and the heads of the linked lists for services, and buffer
 * events. Listener is created and added to the base, services are already set
 * but will be needed by the cb function, adds to the buffer event list. 
 */
void
listen_for_monitors(struct event_base *base, struct evconnlistener *lstnr, list_heads *heads)
{
    char                ip[ip_len], port[port_len];
    int                 i, port_num;
    struct sockaddr_in  m_addr;
    struct in_addr      *inp = (struct in_addr *) malloc(sizeof(struct in_addr));

    if (!parse_address(monitor_addr, ip, port)){
        fprintf(stderr, "Bad address agent unable listen for monitor!!\n");
    } else {
        port_num = atoi(port);
        i = inet_aton(ip, inp);
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = (*inp).s_addr;
        m_addr.sin_port = htons(port_num);
        lstnr = evconnlistener_new_bind(base, monitor_connect_cb, heads, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, 
            -1, (struct sockaddr *) &m_addr, sizeof(m_addr));

        if (!lstnr)
            fprintf(stderr, "Couldn't create listner for monitors.\n");

        // one listener will create a event buffer for each monitor that connects
        // event buffers will all call the same call back, will parse out service and execute the hook sent with service on that service.
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
    evconnlistener_free(listener);
    free_lists_memory(&ser_n_bevs);
    event_base_free(base);
}