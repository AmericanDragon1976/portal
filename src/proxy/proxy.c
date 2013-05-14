#include "portal-proxy.h"
#include "proxySetup.h"
#include "proxy.h"

/* 
 * Call back for information comming in from the monitor, in the buffer info. Function 
 * should recive a c-string that contains the name of the service and the ip address and 
 * port number in the format a.b.c.d:port_num.
 * Call back will verify is the right service, and store the address including port num
 * in the serv member of the service struct, and then if the address is new will terminate
 * all clients connected and free memory. 
 */
void 
monitor_read_cb(struct bufferevent *bev, void *serv_list)
{
    service     *current_serv = (service *) serv_list;
    struct      evbuffer *input = bufferevent_get_input(bev);
    int         len, i, j, k;
    bool        now_addr = false;
    char        *text, temp_name[serv_nm_siz], temp_addr[comp_add_len];

    len = evbuffer_get_length(input);
    text = (char *) malloc(len);
    bzero(text, len); 
    evbuffer_remove(input, text, len); 

    while(current_serv != NULL && current_serv->b_monitor != bev)
        current_serv = current_serv->next;

    if (strcmp(text, "service not found") == 0) {
        fprintf(stderr, "Service %s not found!!\n", current_serv->name);
        return; 
    }

    // parse out name of service, verify correct one, and store the rest in current_serv->serv
    j = 0; k = 0;

    for (i = 0; (isalpha(text[i]) != 0) && (isdigit(text[i]) != 0); i++);   // drop leading whitespace

    // seperate name and address
    for (; i < len; i++){
        if (isdigit(text[i]) != 0) { 
            now_addr = true; 

            if(isalpha(temp_name[k - 1]) == 0) 
                temp_name[k - 1] = '\0';
            else
                temp_name[k] = '\0';
        }

        if (now_addr)
            temp_addr[j++] = text[i];
        else 
            temp_name[k++] = text[i];
    }

    if (strcmp(current_serv->name, temp_name) == 0){

        if (strcmp(temp_addr, current_serv->serv) != 0){
            strcpy(current_serv->serv, temp_addr);
            serv_cli_pair *temp = current_serv->client_list;

            while(current_serv->client_list != NULL){
                current_serv->client_list = temp->next;
                bufferevent_free(temp->b_client);
                bufferevent_free(temp->b_service);
                free(temp);
                temp = current_serv->client_list;
            }
        }
    }
    else
        fprintf(stderr, "ERROR service responce recived from monitor does not correspond to proxy service!\n"); 
}

/* 
 * when triggered by a connecting client determines which service the client was connecting
 * for and connects them to the appropiate service. 
 */
void 
client_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
                   struct sockaddr *address, int socklen, void *ctx)
{ 
    service             *curr_service;
    service_pack        *current_serv_pack = NULL; 
    struct event_base   *base = evconnlistener_get_base(listener);
    struct addrinfo     *hints = NULL;
    struct addrinfo     *serv_addr = NULL;
    char                ip_addr[16] = {'\0'}, port_num[6] = {'\0'};

    curr_service = (service *) ctx;

    while (curr_service != NULL) { 
        if(curr_service->listener == listener) {
            // create a new Service Client Pair and add it to the Client List
            serv_cli_pair *proxy_pair = new_null_serv_cli_pair();

            // create event buffers to proxy client
            proxy_pair->b_client =  bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
            proxy_pair->b_service = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
            current_serv_pack = new_service_package(curr_service, proxy_pair);
            int i = 0; 
            int j = 0; 
            bool port_now = false;

            if (!parse_address(curr_service->serv, ip_addr, port_num))
                fprintf(stderr, "Bad address unable to connect client to %s\n", curr_service->name);
            else {
                hints = set_criteria_addrinfo (); 
                i = getaddrinfo(ip_addr, port_num, hints, &serv_addr);

                if (i != 0) {
                    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
                    return;
                }

                bufferevent_setcb(proxy_pair->b_client, proxy_read_cb, NULL, event_cb, current_serv_pack); 
                bufferevent_enable(proxy_pair->b_client, EV_READ|EV_WRITE);

                if (bufferevent_socket_connect(proxy_pair->b_service, serv_addr->ai_addr, serv_addr->ai_addrlen) != 0) { 
                    fprintf(stderr, "Error Connecting to service\n");
                    char errorMsg[] = "Unable to connect, try again;";
                    bufferevent_write(proxy_pair->b_client, &errorMsg, sizeof(errorMsg));
                    bufferevent_free(proxy_pair->b_client);
                    bufferevent_free(proxy_pair->b_service);
                return;
                }

                bufferevent_setcb(proxy_pair->b_service, proxy_read_cb, NULL, event_cb, current_serv_pack);
                bufferevent_enable(proxy_pair->b_service, EV_READ|EV_WRITE);
    
                // add pair of buffer events to client_list for this service
                proxy_pair->next = curr_service->client_list;
                curr_service->client_list = proxy_pair;
            } 
        }

        curr_service = curr_service->next;
    }
}

/* 
 * Call back for information comming in from either a client or the service they are connected
 * to, function passes the info through. If the service is not there will attempt to 
 * reconnect and send the info. 
 */
void 
proxy_read_cb(struct bufferevent *bev, void *srv_pck) 
{ 
    service_pack        *serv_pack = (service_pack *) srv_pck;
    serv_cli_pair       *cur_pair = serv_pack->pair;
    struct bufferevent  *partner = NULL;
    struct evbuffer     *src, *dst;

    src = bufferevent_get_input(bev); 

    if(bev == cur_pair->b_service)
        partner = cur_pair->b_client;
    else 
        partner = cur_pair->b_service;

    if(!partner){ 
// no partner free the bufferevents free associated memory and remove pair from client_listand return 
        serv_cli_pair *temp = serv_pack->serv->client_list;

        if (temp = serv_pack->pair) {        // the trigger buffer is part of the first client service pair in the list
            serv_pack->serv->client_list = temp->next;
            bufferevent_free(temp->b_client);
            bufferevent_free(temp->b_service);
            free(temp);
            return;
        }
        while (temp != NULL) {

            if (temp->next == serv_pack->pair) {
                cur_pair = temp;
                temp = temp->next;
                cur_pair->next = temp->next;
                bufferevent_free(temp->b_client);
                bufferevent_free(temp->b_service);
                free(temp);
                return;
            }
            temp = temp->next;
        }
        return;
    }
    dst = bufferevent_get_output(partner); 
    evbuffer_add_buffer(dst, src);  
    return;
}


/* 
 * triggered by all event buffer event, reports errors and successful connects. 
 */
void 
event_cb(struct bufferevent *bev, short what, void *ctx)
{ 
    if (what & BEV_EVENT_ERROR) {
        unsigned long err;

        while ((err = (bufferevent_get_openssl_error(bev)))) { printf("1\n");
            const char *msg = (const char*)
                ERR_reason_error_string(err);
            const char *lib = (const char*)
                ERR_lib_error_string(err);
            const char *func = (const char*)
                ERR_func_error_string(err);
            fprintf(stderr,
                "%s in %s %s\n", msg, lib, func);
        }

        if (errno)
            perror("connection error");
    }

    if (what & BEV_EVENT_EOF)
        printf("EOF\n");

    if (what & BEV_EVENT_CONNECTED)
        printf("CONNECTION SUCCESSFUL\n");

    if (what & BEV_EVENT_TIMEOUT)
        printf("TIMEOUT\n");
}

/* 
 * catches interrupt signal and allows the program to cleanup before exiting. 
 */
void 
signal_cb (evutil_socket_t sig, short events, void *user_data) 
{
    struct event_base   *base = (struct event_base *) user_data;
    struct timeval      delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");


    event_base_loopexit(base, &delay);
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
 * allocates memory for a new service_package sets all pointers to the values passed in by parameters and
 * returns a pointer to the new service_package 
 */
void 
free_all_listeners(service *serv_list) 
{
    while (serv_list != NULL) {
        evconnlistener_free(serv_list->listener);
        serv_list = serv_list->next;
    }
}

/* 
 * goes through the list of services, frees memory allocated for each node 
 */
void 
free_all_service_nodes(service *serv_list)
{
    service         *temp_serv = serv_list;
    serv_cli_pair   *temp_pair = NULL;

    while (serv_list != NULL) {
        serv_list = serv_list->next; 

        while (temp_serv->client_list != NULL){
            temp_pair = temp_serv->client_list;
            temp_serv->client_list = temp_pair->next;
            free(temp_pair); 
        }

        free(temp_serv); 
        temp_serv = serv_list;
    }
}

