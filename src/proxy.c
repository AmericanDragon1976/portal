#include "porxy.h"

/* Call back for information comming in from the monitor, in the buffer info. Function 
* should recive a c-string that contains the name of the service and the ip address and 
* port number in the format a.b.c.d:portnum.
* Call back will verify is the right service, and store the address including port num
* in the serv member of the service struct, and then if the address is new will terminate
* all clients connected and free memory. */
static void monitorReadCB(struct bufferevent *bev, void *servList){
    service *currentServ = (service *) servList;
    struct evbuffer *input = bufferevent_get_input(bev);
    int len, i, j, k;
    bool nowAddr = false;
    char *text, tempName[30], tempAddr[22];

    while(currentServ != NULL && currentServ->b_monitor != bev)
        currentServ = currentServ->next;

    // obtain data from buffer event 
    len = evbuffer_get_length(input);
    text = (char *) malloc(len);
    bzero(text, len); 
    evbuffer_remove(input, text, len); 

    // parse out name of service, verify correct one, and store the rest in currentServ->serv
    j = 0; k = 0;
    for (i = 0; !isalpha(text[i]) && !isdigit(text[i]); i++);   // drop leading whitespace
    for (; i < len; i++){                                       // seperate name and address
        if (isdigit(text[i])) { 
            nowAddr = true; 
            if(isalpha(tempName[k - 1])) 
                ;
            else 
                tempName[k - 1] = '\0';
        }
        if (nowAddr)
            tempAddr[j++] = text[i];
        else 
            tempName[k++] = text[i];
    }

    if (!strcmp(currentServ->name, tempName)){
        if (strcmp(tempAddr, currentServ->serv)){
            strcpy(currentServ->serv, tempAddr);
            servCliPair *temp = currentServ->clientList;
            while(currentServ->clientList != NULL){
                currentServ->clientList = temp->next;
                bufferevent_free(temp->b_client);
                bufferevent_free(temp->b_service);
                free(temp);
            }
        }
    }
    else
        fprintf(stderr, "ERROR service responce recived from monitor does not correspond to proxy service!\n"); 
}

/* when triggered by a connecting client determines which service the client was connecting
* for and connects them to the appropiate service. */
static void clientConnectCB(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx){ 
    service *currService;
    currService = (service *) ctx;
    servicePack *currentServPack; 
    currentServPack = (servicePack *) malloc(sizeof(servicePack));
    struct event_base *base = evconnlistener_get_base(listener);
    struct addrinfo hints = {};
    struct addrinfo *servAddr;
    char ipAddr[16] = {'\0'}, portNum[6] = {'\0'};

    while (currService != NULL) { 
        if(currService->listener == listener) {
            // create a new Service Client Pair and add it to the Client List
            servCliPair *proxyPair = (servCliPair *) malloc(sizeof(servCliPair));
            proxyPair->b_client = NULL;
            proxyPair->b_service = NULL;
            proxyPair->next = NULL;

            currentServPack->serv = currService; 

            // create event buffers to proxy client
            proxyPair->b_client =  bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
            proxyPair->b_service = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
            currentServPack->pair = proxyPair;
            int i = 0; 
            int j = 0; 
            bool portNow = false;
            // parse address for ip and socket
            for (i = 0; currService->serv[i] != '\0'; ) {
                if (currService->serv[i] == ':') {
                    i++;
                    portNow = true;
                    j = 0;
                }
                if (portNow == false)
                    ipAddr[j++] = currService->serv[i++];
                else 
                    portNum[j++] = currService->serv[i++];
            }
            portNum[j] = '\0'; 
            // connect to service
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = 0;
            hints.ai_protocol = 0; 
            i = getaddrinfo(ipAddr, portNum, &hints, &servAddr);
            if (i != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
                return;
            }
            bufferevent_setcb(proxyPair->b_client, proxyReadCB, NULL, eventCB, currentServPack); 
            bufferevent_enable(proxyPair->b_client, EV_READ|EV_WRITE);
            if (bufferevent_socket_connect(proxyPair->b_service, servAddr->ai_addr, servAddr->ai_addrlen) != 0) { 
                fprintf(stderr, "Error Connecting to service\n");
                char errorMsg[] = "Unable to connect, try again;";
                bufferevent_write(proxyPair->b_client, &errorMsg, sizeof(errorMsg));
                bufferevent_free(proxyPair->b_client);
                bufferevent_free(proxyPair->b_service);
                return;
            }
            bufferevent_setcb(proxyPair->b_service, proxyReadCB, NULL, eventCB, currentServPack);
            bufferevent_enable(proxyPair->b_service, EV_READ|EV_WRITE);

            // add pair of buffer events to clientList for this service
            proxyPair->next = currService->clientList;
            currService->clientList = proxyPair;
        } 
        currService = currService->next;
    }
}

/* Call back for information comming in from either a client or the service they are connected
* to, function passes the info through. If the service is not there will attempt to 
* reconnect and send the info. */
static void proxyReadCB(struct bufferevent *bev, void *srvPck) { 
    servicePack *servPack = (servicePack *) srvPck;
    servCliPair *curPair = servPack->pair;
    struct bufferevent *partner = NULL;
    struct evbuffer *src, *dst;

    src = bufferevent_get_input(bev); 
    if(bev == curPair->b_service)
        partner = curPair->b_client;
    else 
        partner = curPair->b_service;

    if(!partner){ 
// no partner free the bufferevents free associated memory and remove pair from clientListand return 
        servCliPair *temp = servPack->serv->clientList;
        if (temp = servPack->pair) {        // the trigger buffer is part of the first client service pair in the list
            servPack->serv->clientList = temp->next;
            bufferevent_free(temp->b_client);
            bufferevent_free(temp->b_service);
            free(temp);
            return;
        }
        while (temp != NULL) {
            if (temp->next == servPack->pair) {
                curPair = temp;
                temp = temp->next;
                curPair->next = temp->next;
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


/* triggered by all event buffer event, reports errors and successful connects. */
static void eventCB(struct bufferevent *bev, short what, void *ctx){ 
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

/* catches interrupt signal and allows the program to cleanup before exiting. */
static void signalCB(evutil_socket_t sig, short events, void *user_data) {
    struct event_base *base = (struct event_base *) user_data;
    struct timeval delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");


    event_base_loopexit(base, &delay);
}

/* goes through the list of services, frees the listeners associated with that
* service */
void freeAllListeners(service *servList) {
    while (servList != NULL) {
        evconnlistener_free(servList->listener);
        servList = servList->next;
    }
}

/* goes through the list of services, frees memory allocated for each node */
void freeAllServiceNodes(service *servList){
    service *tempServ = servList;
    servCliPair *tempPair = NULL;

    while (servList != NULL) {
        servList = servList->next; 
        while (tempServ->clientList != NULL){
            tempPair = tempServ->clientList;
            tempServ->clientList = tempPair->next;
            free(tempPair); 
        }
        free(tempServ); 
        tempServ = servList;
    }
}

/*
    service *temp = servList;
    for (; temp != NULL; temp = temp->next){
    printf("name %s\n", temp->name);
    printf("monitor %s\n", temp->monitor);
    printf("listen %s\n", temp->listen);
    printf("serv %s\n", temp->serv);
}
*/