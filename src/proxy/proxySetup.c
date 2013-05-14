#include "portal-proxy.h"
#include "proxySetup.h"
#include "proxy.h"

/* 
 * Verifies the command line arguments. The arguments must include flag -C and 
 * the path/to/comfig.txt. If specific uses of the -C flag or other flags are 
 * added then this function should be altered accordingly.
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
 * Prints to screen the proper syntax for running the program, then exits.
 */
void 
usage()
{
    printf("Usage is as follows: \n");
    printf("    portal-proxy space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    portal-proxy -C ../deps/config.txt\n");
    exit(0);
}

/* 
 * Recives the name and path to athe config file. Returns an int congtining the
 * length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array after it is read in. 
 */
int 
get_config_file_len(char *name) 
{
    int     file_len = 0;
    char    *buffer;
    FILE    *file_ptr;

    file_ptr = fopen(name, "r");
    if (file_ptr == NULL) {
        fprintf(stderr, "Unable to open config file. chech file name and path!\n");
        exit(0);
    }

    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr);
    fclose(file_ptr);

    return (file_len);
}

/* 
 * Recives the name a path to the config file, and a int containing the length 
 * of the file. Returns pointer to a char array holding contents of the file.  
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
    return buffer;
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
 * Recives a char pointer to the buffer containing the config file text. 
 * Returns a pointer to the head of a linked list of services. 
 */
service* 
parse_config_file(char *buff, int len)
{
    service     *list_head = new_null_service_node();
    service     *current_record = list_head;
    char        serIdent[] = "service";

    int j = 0;
    int i = 0;

    while (i < len){
        for (j = 0; j < sizeof(serIdent) - 1; j++)  // read Identifier "service"
            serIdent[j] =  buff[i++];

        i++;                                        // advance past white space

        if (strcmp(serIdent, "service")){           // returns 0 (false) only if they are equal
            fprintf(stderr, "Config file Corrupted. \n");
            exit(0);
        }

        j = 0;

        while (buff[i] != '\n') 
            current_record->name[j++] = buff[i++];   // read service name

        current_record->name[j] = '\0';
        i++;                                        // disgard \n

        for (; buff[i++] != 'n';);                  // find end of identifier "listen"

        i++;                                        // advance beyond white space
        j = 0;

        while (buff[i] != '\n')
            current_record->listen[j++] = buff[i++]; // read listen address

        current_record->listen[j] = '\0';

        for (;buff[i++] != 'r';)                    // find end of identifier "monitor"
            ;                   
        i++;                                        // advance beyond white space
        j = 0;

        while (i < len && buff[i] != '\n')
            current_record->monitor[j++] = buff[i++];// read monitor address

        current_record->monitor[j] = '\0';

        if (i < len){
            current_record->next = new_null_service_node();

            current_record = current_record->next;
            for (; buff[i++] != 's';)              // advance to next record
                ;

            i--;
        }
    }
    return list_head;
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
        bool port_now = false;
        int i = 0; 
        int j = 0; 

        if (!parse_address(serv_list->listen, ip_addr, port_num))
            fprintf(stderr, "Bad address unable listen for clients for service %s\n", serv_list->name);
        else {
            port_no = atoi(port_num);
            i = inet_aton(ip_addr, inp); 
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
