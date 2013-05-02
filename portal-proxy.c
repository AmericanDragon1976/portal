#include "portal-proxy.h"

/*********************************************************************
* called by toEvent every time it times out, 
* will eventually be used to check if the server is still up. 
*********************************************************************/
static void timeout_cb(evutil_socket_t fd, short what, void *arg) { 
/*  service *test = (service *) arg;
    bufferevent_write(test->b_monitor, test->name, sizeof(test->name));
    printf("timeout_cb called\n");
    printf("Server up\n");
*/}

int main(int argc, char **argv) {
    bool cFlag = false;
    bool flagsSet = false;
    int i = 0;
    service *serviceList = NULL;
    char fileName[100];
    FILE *filePointer;
    long fileSize;
    char *fileBuffer;
    size_t result;
    struct event_base *base;
    struct event *signal_event;

    if (argc <3)
        usage();
    char *flag = argv[1];

// checking flags set 
// Currently only -C flag is used but others could be added
// later by adding additional if statements before checking if flags
// have been set at the end of the for loop.
    for (i = 1; i < argc - 1; i++) { 
        if (!strcmp(argv[i], "-C")) {
            cFlag = true;
            flagsSet = true;
        }
        if (!flagsSet)
            usage();
    }

// read config.txt, parse, and creat linked list of services
    strcpy(fileName, argv[2]);
    fileBuffer = readFile(fileName, &fileSize);
    serviceList = parseConfigFile(fileBuffer, fileSize);    
    free(fileBuffer);
 
// create event base
    base = event_base_new();

// connect to monitors
    initServices(base, serviceList);

// create listeners to accept clients for each service
    initServiceListeners(base, serviceList); 

struct timeval five_seconds;
five_seconds.tv_sec = 5;
five_seconds.tv_usec = 0;
struct event *toEvent; // time out event do this ever so often
toEvent = event_new(base, -1, EV_PERSIST, timeout_cb, serviceList);
event_add(toEvent, &five_seconds);

// create listener to handle proper shutdown in case of interrupt signal
    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *) base);
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }

    event_base_dispatch(base);
    freeAllListeners(serviceList);
    freeAllServiceNodes(serviceList);
    event_base_free(base);
}

/******************************************************************************************
* prints to screen proper syntax for running the program, then exits
******************************************************************************************/
static void usage(){
    printf("Usage is as follows: \n");
    printf("    portal-proxy space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    portal-proxy -C deps/config.txt\n");
    exit(0);
}

/******************************************************************************************
* Read a file, Recive name of file with length of 100, and long var (len) by referance
* return pointer to c string (buffer) holding contents of the file, int will now contain
* length of the of the buffer. len needs set so that the info is avaliable later. 
******************************************************************************************/
static char* readFile(char *name, long *len){
    int nameLength = 100;
    char *buffer;
    FILE *filePtr;
    size_t result;

    filePtr = fopen(name, "r");
    if (filePtr == NULL) {
        fprintf(stderr, "Unable to open file, check file name and path!\n");
        exit(0);
    }

    fseek(filePtr, 0, SEEK_END);
    *len = ftell(filePtr);
    rewind(filePtr);
    buffer = (char *) malloc(sizeof(char)*(*len));
    if (buffer == NULL) {
        fprintf(stderr, "Memory Error, creation of File Buffer Failed!\n");
        exit(0);
    }
    result = fread(buffer, 1, *len, filePtr);
    if (result != *len) {
        fprintf(stderr, "Error reading file.\n");
        exit(0);
    }
    fclose(filePtr);
    return buffer;
}

/******************************************************************************************
* recives a service pointer to buffer containing config file
* returns pointer that is the head of a list of services. 
******************************************************************************************/
static service* parseConfigFile(char *buff, long len){
    service *listHead = (service *) calloc(1, sizeof(service));
    listHead->next = NULL;
    listHead->clientList = NULL;
    service *currentRecord = listHead;
    char serIdent[] = "service";
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
            currentRecord->name[j++] = buff[i++];   // read service name
        currentRecord->name[j] = '\0';
        i++;                                        // disgard \n
        for (; buff[i++] != 'n';);                  // find end of identifier "listen"
        i++;                                        // advance beyond white space
        j = 0;
        while (buff[i] != '\n')
            currentRecord->listen[j++] = buff[i++]; // read listen address
        currentRecord->listen[j] = '\0';
        for (;buff[i++] != 'r';);                   // find end of identifier "monitor"
        i++;                                        // advance beyond white space
        j = 0;
        while (i < len && buff[i] != '\n')
            currentRecord->monitor[j++] = buff[i++];// read monitor address
        currentRecord->monitor[j] = '\0';

        if (i < len){
            currentRecord->next = (service *) calloc(1, sizeof(service));
            currentRecord = currentRecord->next;
            currentRecord->next = NULL;
            currentRecord->clientList = NULL;
            for (; buff[i++] != 's';);              // advance to next record
            i--;
        }
    }
    return listHead;
}

/******************************************************************************************
* goes through list of services and for each service:
*   connects to monitor
*   gets service addr    
******************************************************************************************/
static void initServices(struct event_base *eBase, service *serviceList) {
    service *servList = (service *) serviceList;
    struct addrinfo *server;
    struct addrinfo hints = {};

    while (servList != NULL) {
        // parse monitor addr for ip and port numbers 
        char ipAddr[16], portNum[6];
        bool portNow = false;
        int i = 0; 
        int j = 0; 
        for (i = 0; servList->monitor[i] != '\0'; ) {
            if (servList->monitor[i] == ':') {
                i++;
                ipAddr[j] = '\0';
                portNow = true;
                j = 0;
            }
            if (portNow == false)
                ipAddr[j++] = servList->monitor[i++];
            else 
                portNum[j++] = servList->monitor[i++];
        }
        portNum[j] = '\0';

        // get the address info for the ip address and port
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0; 
        i = getaddrinfo(ipAddr, portNum, &hints, &server); 
        if (i != 0){                                                         
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
            exit(0);
        } 
        // create a bufferevent to listen to monitor, connect to monitor and send service name
        // as a request for current ip address and port for that service
        servList->b_monitor = bufferevent_socket_new(eBase, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST); 
        bufferevent_setcb(servList->b_monitor, monitorRead, NULL, cbEvent, serviceList); 
        if(bufferevent_socket_connect(servList->b_monitor, server->ai_addr, server->ai_addrlen) != 0) { 
            fprintf(stderr, "Error connecting to monitor\n"); 
            bufferevent_free(servList->b_monitor); 
        } 
        bufferevent_enable(servList->b_monitor, EV_READ|EV_WRITE);
        bufferevent_write(servList->b_monitor, servList->name, sizeof(servList->name));

        servList = servList->next;
    }

    return;
}


/*****************************************************************************************
* goes through the list of services, creats a listener to accept new clients
* for each service in the list
*****************************************************************************************/
static void initServiceListeners(struct event_base *eBase, service *servList) {
    int portno;
    struct sockaddr_in serv_addr;
    struct in_addr *inp;
    inp = (struct in_addr *) malloc (sizeof(struct in_addr));

    while (servList != NULL) {
        char ipAddr[16], portNum[6];
        bool portNow = false;
        int i = 0; 
        int j = 0; 
        for (i = 0; servList->listen[i] != '\0'; ) {
            if (servList->listen[i] == ':') {
                i++;
                ipAddr[j] = '\0';
                portNow = true;
                j = 0;
            }
            if (portNow == false)
                ipAddr[j++] = servList->listen[i++];
            else 
                portNum[j++] = servList->listen[i++];
        } 
        portNum[j] = '\0';
        portno = atoi(portNum);
        i = inet_aton(ipAddr, inp); 
        memset(&serv_addr,0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = (*inp).s_addr; 
        serv_addr.sin_port = htons(portno); 
        servList->listener = evconnlistener_new_bind(eBase, onClientConnect, servList, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
        if (!servList->listener)
            printf("Couldn't create Listener\n");
        servList = servList->next;
    }
}

/*****************************************************************************************
* when triggered by a connecting client determines which service the client was connecting
* for and connects them to the appropiate service.
*****************************************************************************************/
static void onClientConnect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx){ 
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
            bufferevent_setcb(proxyPair->b_client, proxyRead, NULL, cbEvent, currentServPack); 
            bufferevent_enable(proxyPair->b_client, EV_READ|EV_WRITE);
            if (bufferevent_socket_connect(proxyPair->b_service, servAddr->ai_addr, servAddr->ai_addrlen) != 0) { 
                fprintf(stderr, "Error Connecting to service\n");
                char errorMsg[] = "Unable to connect, try again;";
                bufferevent_write(proxyPair->b_client, &errorMsg, sizeof(errorMsg));
                bufferevent_free(proxyPair->b_client);
                bufferevent_free(proxyPair->b_service);
                return;
            }
            bufferevent_setcb(proxyPair->b_service, proxyRead, NULL, cbEvent, currentServPack);
            bufferevent_enable(proxyPair->b_service, EV_READ|EV_WRITE);

            // add pair of buffer events to clientList for this service
            proxyPair->next = currService->clientList;
            currService->clientList = proxyPair;
        } 
        currService = currService->next;
    }
}

/*****************************************************************************************
* goes through the list of services, frees the listeners associated with that
* service 
*****************************************************************************************/
void freeAllListeners(service *servList) {
    while (servList != NULL) {
        evconnlistener_free(servList->listener);
        servList = servList->next;
    }
}

/*****************************************************************************************
* goes through the list of services, frees memory allocated for each node 
*****************************************************************************************/
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

/*****************************************************************************************
* catches interrupt signal and allows the program to cleanup before exiting.
*****************************************************************************************/
static void signal_cb(evutil_socket_t sig, short events, void *user_data) {
    struct event_base *base = (struct event_base *) user_data;
    struct timeval delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");


    event_base_loopexit(base, &delay);
}

/*****************************************************************************************
* triggered by all event buffer event, reports errors and successful connects. 
*****************************************************************************************/
static void cbEvent(struct bufferevent *bev, short what, void *ctx){ 
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

/*****************************************************************************************
* Call back for information comming in from the monitor, in the buffer info function 
* should recive a c-string that contains the name of the service and the ip address and 
* port number in the format a.b.c.d:portnum.
* Call back will verify is the right service, and store the address including port num
* in the serv member of the service struct, and then call function to reset connections
* for all clients connected to that service
*****************************************************************************************/
static void monitorRead(struct bufferevent *bev, void *servList){
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

/*****************************************************************************************
* Call back for information comming in from either a client or the service they are connected
* to, function passes the info through. If the service is not there will attempt to 
* reconnect and send the info. 
*****************************************************************************************/
static void proxyRead(struct bufferevent *bev, void *srvPck) { 
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
/*
    service *temp = servList;
    for (; temp != NULL; temp = temp->next){
    printf("name %s\n", temp->name);
    printf("monitor %s\n", temp->monitor);
    printf("listen %s\n", temp->listen);
    printf("serv %s\n", temp->serv);
}
*/