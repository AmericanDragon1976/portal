#include "proxy.h"
#include "proxy_config.h"
#include "proxy_events.h"

/* 
 * Call back for information comming in from the monitor, in the buffer info. Function 
 * should recive a c-string that contains the name of the service and the ip address and 
 * port number in the format a.b.c.d:port_number.
 * Call back will verify is the right service, and store the address including port num
 * in the serv member of the service struct, and then if the address is new will terminate
 * all clients connected and free memory. 
 */
void 
monitor_read_cb(struct bufferevent *bev, void passed_svc_list[])
{
    service     *svc_list = (service *)passed_svc_list;
    int         current_svc = 0;
    struct      evbuffer *input = bufferevent_get_input(bev);
    int         len, i, j, k;
    bool        now_addr = false;
    char        *text, temp_name[svc_name_len], temp_address[complete_address_len];

    len = evbuffer_get_length(input);
    text = (char *) malloc(len);
    bzero(text, len); 
    evbuffer_remove(input, text, len); 

    while(current_svc < list_len && svc_list[current_svc].monitor_buffer_event != bev)
        current_svc++;

    if (strcmp(text, "service not found") == 0) {
        fprintf(stderr, "Service %s not found!!\n", svc_list[current_svc].name);
        return; 
    }

    // parse out name of service, verify correct one, and store the rest in svc_list[current_svc].svc
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
            temp_address[j++] = text[i];
        else 
            temp_name[k++] = text[i];
    }

    if (current_svc < list_size && strcmp(svc_list[current_svc].name, temp_name) == 0){

        if (strcmp(temp_address, svc_list[current_svc].svc) != 0){
            strcpy(svc_list[current_svc].svc, temp_address);
            free_pair_list(svc_list[current_svc].client_list);
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
client_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx)
{ 
    service             *svc_list;
    int                 current_svc = 0;
    service_pack        *current_svc_pack = NULL; 
    struct event_base   *base = evconnlistener_get_base(listener);
    struct addrinfo     *hints = NULL;
    struct addrinfo     *svc_address = NULL;
    char                ip_address[16] = {'\0'}, port_number[6] = {'\0'};

    svc_list = (service *) ctx;

    while (current_svc < list_len && strcmp(svc_list[current_svc].name, "none") != 0) { 
        if(svc_list[current_service].listener == listener) {
            // create a new Service Client Pair and add it to the Client List
            svc_client_pair *proxy_pair = new_null_svc_client_pair();

            // create event buffers to proxy client
            proxy_pair->client_buffer_event =  bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
            proxy_pair->service_buffer_event = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
            current_svc_pack = new_svcice_package(current_service, proxy_pair);
            int i = 0; 
            int j = 0; 
            bool port_now = false;

            if (!parse_address(current_service->svc, ip_address, port_number))
                fprintf(stderr, "Bad address unable to connect client to %s\n", current_service->name);
            else {
                hints = set_criteria_addrinfo (); 
                i = getaddrinfo(ip_address, port_number, hints, &svc_address);

                if (i != 0) {
                    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
                    return;
                }

                bufferevent_setcb(proxy_pair->client_buffer_event, proxy_read_cb, NULL, event_cb, current_svc_pack); 
                bufferevent_enable(proxy_pair->client_buffer_event, EV_READ|EV_WRITE);

                if (bufferevent_socket_connect(proxy_pair->service_buffer_event, svc_address->ai_addr, svc_address->ai_addrlen) != 0) { 
                    fprintf(stderr, "Error Connecting to service\n");
                    char error_message[] = "Unable to connect, try again;";
                    bufferevent_write(proxy_pair->client_buffer_event, &error_message, sizeof(error_message));
                    bufferevent_free(proxy_pair->client_buffer_event);
                    bufferevent_free(proxy_pair->service_buffer_event);
                return;
                }

                bufferevent_setcb(proxy_pair->service_buffer_event, proxy_read_cb, NULL, event_cb, current_svc_pack);
                bufferevent_enable(proxy_pair->service_buffer_event, EV_READ|EV_WRITE);
    
                // add pair of buffer events to client_list for this service
                proxy_pair->next = current_service->client_list;
                current_service->client_list = proxy_pair;
            } 
        }

        current_service = current_service->next;
    }
}

/* 
 * Call back for information comming in from either a client or the service they are connected
 * to, function passes the info through. If the service is not there will attempt to 
 * reconnect and send the info. 
 */
void 
proxy_read_cb(struct bufferevent *buffer_event, void *srv_pck) 
{ 
    service_pack        *svc_pack = (service_pack *) srv_pck;
    svc_client_pair     *current_pair = svc_pack->pair;
    struct bufferevent  *partner = NULL;
    struct evbuffer     *source, *destination;

    source = bufferevent_get_input(buffer_event); 

    if(buffer_event == current_pair->service_buffer_event)
        partner = current_pair->client_buffer_event;
    else 
        partner = current_pair->service_buffer_event;

    if(!partner){ 
// no partner free the bufferevents free associated memory and remove pair from client_listand return 
        svc_client_pair *temp = svc_pack->svc->client_list;

        if (temp = svc_pack->pair) {        // the trigger buffer is part of the first client service pair in the list
            svc_pack->svc->client_list = temp->next;
            bufferevent_free(temp->client_buffer_event);
            bufferevent_free(temp->service_buffer_event);
            free(temp);
            return;
        }
        while (temp != NULL) {

            if (temp->next == svc_pack->pair) {
                current_pair = temp;
                temp = temp->next;
                current_pair->next = temp->next;
                bufferevent_free(temp->client_buffer_event);
                bufferevent_free(temp->service_buffer_event);
                free(temp);
                return;
            }
            temp = temp->next;
        }
        return;
    }
    destination = bufferevent_get_output(partner); 
    evbuffer_add_buffer(destination, source);  
    return;
}


/* 
 * triggered by all event buffer event, reports errors and successful connects. 
 */
void 
event_cb(struct bufferevent *buffer_event, short what, void *ctx)
{ 
    if (what & BEV_EVENT_ERROR) {
        unsigned long err;

        while ((err = (bufferevent_get_openssl_error(buffer_event)))) { printf("1\n");
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
