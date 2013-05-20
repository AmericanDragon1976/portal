#include "portal-agent.h"
#include "agentSetup.h"
#include "agent.h"

/* 
 * Verifys the command line areguament for the monitor, returns ture if correct 
 * argumets are supplied and false otherwise. 
 */
bool 
verify_comnd_ln_args(int argc, char **argv) 
{
    if (argc < 3)         
        return (false);

    if (strcmp(argv[1], "-C") != 0) // if other flags added or effect of -C flag changes alter here. 
        return (false);

    return (true);
}

/* 
 * Returns the length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array once it is read in. 
 */
int 
get_config_file_len(char *name)
{
    int     file_len = 0;
    char    *buffer;
    FILE    *file_ptr;

    file_ptr = fopen(name, "r");

    if (file_ptr == NULL) {
        fprintf(stderr, "Unable to open config file. check file name and path!\n");
        exit(0);
    }

    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr);
    fclose(file_ptr);

	return (file_len);
}

/* 
 * Read a file, Recive name of file and length. 
 * Return pointer to char array (buffer) holding contents of the file. 
 */
char* 
read_file(char *name, int len) 
{
    char    *buffer = (char *) malloc(sizeof(char) * (len));
    FILE    *file_ptr;
    size_t  result;

    file_ptr = fopen(name, "r");

    if (file_ptr == NULL) {
        fprintf(stderr, "Unable to open file, check file name and path!\n");
        exit(0);
    }

    if (buffer == NULL) {
        fprintf(stderr, "Memory Error, creation of File Buffer Failed!\n");
        exit(0);
    }

    result = fread(buffer, 1, len, file_ptr);

    if (result != len) {
        fprintf(stderr, "Error reading file.\n");
        exit(0);
    }

    fclose(file_ptr);
    return (buffer);
}

/*
 * Allocates memory for a new serv_lst, sets all pointer members to NULL, and
 * returns a pointer to it. 
 */
serv_lst*
new_null_serv_lst()
{
    serv_lst        *nw_serv_lst = (serv_lst *) malloc(sizeof(serv_lst));

    nw_serv_lst->next = NULL;
    nw_serv_lst->cmd_lst = NULL;

    return (nw_serv_lst);
}

/*
 * Allocates memory for a new serv_lst, sets all pointer members to the value 
 * passed in and returns a pointer to it. 
 */
serv_lst*
new_serv_lst(serv_lst *nxt, hook_path_pair *cmd_lst_head)
{
    serv_lst        *nw_serv_lst = (serv_lst *) malloc(sizeof(serv_lst));

    nw_serv_lst->next = nxt;
    nw_serv_lst->cmd_lst = cmd_lst_head;

    return (nw_serv_lst);
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
 * Recives a service pointer to the buffer containing config file
 * Returns a service list, each service has a name and a list of 
 * commands it can use to test the service. 
 */
serv_lst* 
parse_config_file(char *buff, int len) 
{
    serv_lst    *head = NULL; 
    serv_lst    *curr_node = NULL;
    char        text[file_nm_len] = {};
    char        hook_nm[hook_len];
    char        hook_path[file_nm_len];
    int         i, j;

    if(buff == NULL || len < 1){
        fprintf(stderr, "Errer reading config file!!\n");
        free(head);
        exit(0);
    }

    j = 0; 
 
    for (i = 0; buff[i] == ' '; i++)                  // advance beyond any leading whitespace
        ;

    bzero(&text, file_nm_len);

    while (buff[i++] != ' ')
        text[j++] = buff[i - 1];            // read in word service
    text[j] = '\0';

    if (strcmp(text, "service") != 0) { 
        fprintf(stderr, "fatal error, bad config file.\n");
        exit(0);  
    }

    while (i < len) {
        curr_node = new_null_serv_lst();
        j = 0;
       bzero(&text, file_nm_len);

        while (buff[i++] != '\n')
            text[j++] = buff[i - 1];

        text[j] = '\0';
        strcpy(curr_node->name, text);
        bzero(&text, file_nm_len);
        j = 0; 

        for (; buff[i] == ' '; i++)
            ;

        while(buff[i++] != ' ')
            text[j++] = buff[i - 1];

            text[j] = '\0';

        while (strcmp(text, "service") != 0){
            hook_path_pair *temp_hook_lst = new_null_hook_path_pair();

            strcpy(temp_hook_lst->hook, text); 
            j = 0;
           bzero(&text, file_nm_len);

            while(i < len && buff[i++] != '\n'){
                text[j++] = buff[i - 1];
            } 

            text[j] = '\0';
            strcpy(temp_hook_lst->path, text); 
            j = 0;
            bzero(&text, file_nm_len);

            temp_hook_lst->next = curr_node->cmd_lst;
            curr_node->cmd_lst = temp_hook_lst;

            if (i < len){
                for (; buff[i] == ' '; i++)
                    ;

                while(buff[i++] != ' ')
                    text[j++] = buff[i -1];

                text[j] = '\0';
            } else {
                break;
            }
        }
        curr_node->next = head;
        head = curr_node;
    }
    return (head);
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