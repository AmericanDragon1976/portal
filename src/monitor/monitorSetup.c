#include "portal-monitor.h"
#include "monitorSetup.h"
#include "monitor.h"

/* 
 * Verifys the command line areguament for the monitor, returns ture if correct 
 * argumets are supplied and false otherwise. 
 */
bool 
verify_comnd_args(int argc, char **argv) 
{
    if (argc < 2)
        return (false);
    else 
        return (true);
}

/* 
 * returns the length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array once it is read in. 
 */
int 
get_config_file_len(char *name) 
{
    int     fileLen = 0;
    int     nameLength = 100;
    char    *buffer;
    FILE    *file_ptr;

    file_ptr = fopen(name, "r");

    if (file_ptr == NULL) {
        fprintf(stderr, "Unable to open config file. check file name and path!\n");
        exit(0);
    }

    fseek(file_ptr, 0, SEEK_END);
    fileLen = ftell(file_ptr);
    fclose(file_ptr);

    return (fileLen);
}

/* 
 * Read a file, Recive name of file with length of <= 100, and int var (len) by referance
 * return pointer to c string (buffer) holding contents of the file, int will now contain
 * length of the of the buffer. len needs set so that the info is avaliable later. 
 */
char* 
read_file(char *name, int len)
{
    int     nameLength = 100;
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
 * recives a service pointer to buffer containing config file
 * returns pointer that is the head of a list of services. 
 */
moni_serv* 
parse_config_file(char *buff, int len) 
{ 
    moni_serv   *listHead = new_null_moni_serv_node();
    moni_serv   *currentRecord = listHead;
    char        serIdent[] = "service";
    int         j = 0;
    int         i = 0;

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
            currentRecord->name[j++] = buff[i++];   // read service name
        
        currentRecord->name[j] = '\0';
        i++;                                        // disgard \n
        
        for (; buff[i++] != 'y';)                   // find end of identifier "proxy"
            ;
        
        i++;                                        // advance beyond white space
        j = 0;

        while (buff[i] != '\n')
            currentRecord->proxyAddr[j++] = buff[i++]; // read proxy address

        currentRecord->proxyAddr[j] = '\0';

        for (;buff[i++] != 't';)                    // find end of identifier "agent"
            ;

        i++;                                        // advance beyond white space
        j = 0;

        while (i < len && buff[i] != '\n')
            currentRecord->agentAddr[j++] = buff[i++];// read agent address

        currentRecord->agentAddr[j] = '\0';

        if (i < len){
            currentRecord->next = new_null_moni_serv_node();
            currentRecord = currentRecord->next;

            for (; buff[i++] != 's';)               // advance to next record
                ;
            i--;
        }
    }
}

/* 
 * creates a new instance of a moni_serv struct with all pointers inicilized to NULL 
 */
moni_serv* 
new_null_moni_serv_node() 
{
    moni_serv   *nw_moni_serv = (moni_serv *) malloc(sizeof(moni_serv));

    nw_moni_serv->listener = NULL;
    nw_moni_serv->next = NULL;
    nw_moni_serv->b_proxy = NULL;
    nw_moni_serv->b_agent = NULL;

    return (nw_moni_serv);
}

/* 
 * creates a new instance of moni_serve struct with pointers set to the passed in values 
 */
moni_serv* 
new_moni_serv_node (struct evconnlistener* lstnr, moni_serv* service, 
    struct bufferevent *bevProxy, struct bufferevent *bevAgent) 
{
    moni_serv   *nw_moni_serv = (moni_serv *) malloc(sizeof(moni_serv));

    nw_moni_serv->listener = lstnr;
    nw_moni_serv->next = service;
    nw_moni_serv->b_proxy = bevProxy;
    nw_moni_serv->b_agent = bevAgent;

    return (nw_moni_serv);
}

/* 
 * Takes an adrress in the form a.b.c.d:port_number and parses it storing the ip
 * address and port number in the approiate char arrays, addr_to_parse[22], ip_addr[16] 
 * and port_num[6], returns true if successful otherwise returns false 
 */
bool 
parse_address (char *addr_to_parse, char *ip_addr, char* port_num) 
{
    int     i, j;
    bool    port_now = false;

    j = 0;

    if ( addr_to_parse == NULL)
        return (port_now);

    for (i = 0; i < 22; ){
        if (addr_to_parse[i] == ':') {
            i++;
            ip_addr[j] = '\0';
            port_now = true; 
            j = 0;
        }
        if (port_now == false)
            ip_addr[j++] = addr_to_parse[i++];
        else 
            port_num[j++] = addr_to_parse[i++];
    }
    port_num[j] = '\0';

    return (port_now);
}

/* 
 * Connects to the agent for each service 
 */
void 
contact_agents (struct event_base *base, moni_serv *s_list) 
{
    moni_serv           *curr_serv = s_list;
    struct addrinfo     *agent_server = NULL;
    struct addrinfo     *hints = NULL;

    while (curr_serv != NULL) {
        char    agent_ip[16], agent_port[6]; 
        int     i = 0;
        int     j = 0;

        if (!parse_address(curr_serv->agentAddr, agent_ip, agent_port)) {
            fprintf(stderr, "Bad address unable to connect to agent for %s\n", curr_serv->name);
        }
        else {
            hints = set_criteria_addrinfo();
            i = getaddrinfo(agent_ip, agent_port, hints, &agent_server);

            if (i != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
                curr_serv = curr_serv->next;
                continue;
            }

            curr_serv->b_agent = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
            bufferevent_setcb(curr_serv->b_agent, NULL, NULL, NULL, s_list);       //add actuall call back functions in place of NULL as they are written (read, write, event)

            if(bufferevent_socket_connect(curr_serv->b_agent, agent_server->ai_addr, agent_server->ai_addrlen) != 0) { 
                fprintf(stderr, "Error connecting to agent\n"); 
                bufferevent_free(curr_serv->b_agent);
            } 

            bufferevent_enable(curr_serv->b_agent, EV_READ|EV_WRITE);
            bufferevent_write(curr_serv->b_agent, curr_serv->name, sizeof(curr_serv->name));
        }

        curr_serv = curr_serv->next;
    }
}

/* 
 * creates a listener for each servics to listen for the proxys and establish the buffer events for them at that time 
 */
void 
listen_for_proxys(struct event_base *base, moni_serv *s_list)
{

}

/*
 * sets the information in in and addrinfo structure to be used as the critera for stuctrue passed into getaddrinfo() 
 */
struct addrinfo* 
set_criteria_addrinfo () 
{
    struct addrinfo     *hints = (struct addrinfo *) malloc(sizeof(struct addrinfo));

    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = 0;
    hints->ai_protocol = 0;

    return (hints);
}
        // get current address for service(s) being monitored from the agent(s)
        // establish listener(s) for proxy agent(s) 