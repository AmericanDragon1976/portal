#include "setup.h"

/* prints to screen proper syntax for running the program, then exits */
static void usage(){
    printf("Usage is as follows: \n");
    printf("    portal-proxy space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    portal-proxy -C ../deps/config.txt\n");
    exit(0);
}

/*  Read a file, Recive name of file with length of 100, and long var (len) by referance
* return pointer to c string (buffer) holding contents of the file, int will now contain
* length of the of the buffer. len needs set so that the info is avaliable later. */
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

/* recives a service pointer to buffer containing config file
* returns pointer that is the head of a list of services. */
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

/* goes through list of services and for each service:
*   connects to monitor
*   requests service addr */
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
        bufferevent_setcb(servList->b_monitor, monitorReadCB, NULL, eventCB, serviceList); 
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

/* goes through the list of services, creats a listener to accept new clients
* for each service in the list */
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
        servList->listener = evconnlistener_new_bind(eBase, clientConnectCB, servList, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
        if (!servList->listener)
            printf("Couldn't create Listener\n");
        servList = servList->next;
    }
}


