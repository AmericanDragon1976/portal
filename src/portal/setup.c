#include "portal-proxy.h"
#include "setup.h"
#include "proxy.h"

/* verifies command line arguments, must have flag -C and path/to/comfig.txt if 
* specific uses of the -C flag or other flags are added this function should be 
* altered accordingly */
bool verifyComndLnArgs(int argc, char **argv) {

    if (argc < 3)               // 3 args are program name, flag, path to config file. 
        return false;

    if (!strcmp(argv[1], "-C")) // if other flags added or effect of -C flag changes alter here. 
        return false;

    return true;
}

/* prints to screen proper syntax for running the program, then exits */
void usage(){
    printf("Usage is as follows: \n");
    printf("    portal-proxy space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    portal-proxy -C ../deps/config.txt\n");
    exit(0);
}

/* returns the length of the config file. There is a good probility that 
* the file will contain 0's and so strlen() can not be used to determine 
* the length of the char array once it is read in. */
int getConfigFileLen(char *name) {
    int fileLen = 0;
    int nameLength = 100;
    char *buffer;
    FILE *filePtr;

    filePtr = fopen(name, "r");
    if (filePtr == NULL) {
        fprintf(stderr, "Unable to open config file. chech file name and path!\n");
        exit(0);
    }

    fseek(filePtr, 0, SEEK_END);
    fileLen = ftell(filePtr);
    fclose(filePtr);

    return fileLen;
}

/*  Read a file, Recive name of file with length of 100, and int var (len) by referance
* return pointer to c string (buffer) holding contents of the file, int will now contain
* length of the of the buffer. len needs set so that the info is avaliable later. */
char* readFile(char *name, int len){
    int nameLength = 100;
    char *buffer = (char *) malloc(sizeof(char) * (len));
    FILE *filePtr;
    size_t result;

    filePtr = fopen(name, "r");
    if (filePtr == NULL) {
        fprintf(stderr, "Unable to open file, check file name and path!\n");
        exit(0);
    }

    if (buffer == NULL) {
        fprintf(stderr, "Memory Error, creation of File Buffer Failed!\n");
        exit(0);
    }
    result = fread(buffer, 1, len, filePtr);
    if (result != len) {
        fprintf(stderr, "Error reading file.\n");
        exit(0);
    }
    fclose(filePtr);
    return buffer;
}

/* allocates memory for a new service node and inicilizes all pointer members to null 
* returns a pointer to this new node */
service* newNullServiceNode () {
    service *nwNode = (service *) calloc(1, sizeof(service));
    nwNode->next = NULL;
    nwNode->listener = NULL;
    nwNode->bMonitor = NULL;
    nwNode->clientList = NULL;

    return nwNode;
}

/* allocates memory for a new wervice node and inicilizes all pointer menters to the 
* values passed in via parameters */
service* newServiceNode (service *nxt, struct evconnlistener *lstnr, struct bufferevent *bevm, servCliPair *scp) {
    service *nwNode = (service *) calloc(1, sizeof(service));
    nwNode->next = nxt;
    nwNode->listener = lstnr;
    nwNode->bMonitor = bevm;
    nwNode->clientList = scp;

    return nwNode;
}

/* recives a service pointer to buffer containing config file
* returns pointer that is the head of a list of services. */
service* parseConfigFile(char *buff, int len){
    service *listHead = newNullServiceNode();
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
            currentRecord->next = newNullServiceNode();
            currentRecord = currentRecord->next;
            for (; buff[i++] != 's';);              // advance to next record
            i--;
        }
    }
    return listHead;
}

/* goes through list of services and for each service:
*   connects to monitor
*   requests service addr */
void initServices(struct event_base *eBase, service *serviceList) {
    service *servList = (service *) serviceList;
    struct addrinfo *server;
    struct addrinfo hints = {};

    while (servList != NULL) {
        char ipAddr[16], portNum[6];
        int i = 0; 
        int j = 0; 

        if (!parseAddress(servList->monitor, ipAddr, portNum))
            fprintf(stderr, "Bad address unable to connect to monitor for %s\n", servList->name);
        else {
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
            servList->bMonitor = bufferevent_socket_new(eBase, -1, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST); 
            bufferevent_setcb(servList->bMonitor, monitorReadCB, NULL, eventCB, serviceList); 
            if(bufferevent_socket_connect(servList->bMonitor, server->ai_addr, server->ai_addrlen) != 0) { 
                fprintf(stderr, "Error connecting to monitor\n"); 
                bufferevent_free(servList->bMonitor); 
            } 
            bufferevent_enable(servList->bMonitor, EV_READ|EV_WRITE);
            bufferevent_write(servList->bMonitor, servList->name, sizeof(servList->name));
        }
        servList = servList->next;
    }

    return;
}

/* Takes an adrress in the form a.b.c.d:portnumber and parses it storing the ip
* address and port number in the approiate char arrays, addrToParse[22], ipAddr[16] 
* and portNum[6], returns true if successful otherwise returns false */
bool parseAddress(char *addrToParse, char *ipAddr, char* portNum) {
    int i, j;
    j = 0;
    bool portNow = false;

    if ( addrToParse == NULL)
        return portNow;

    for (i = 0; i < 22; ){
        if (addrToParse[i] == ':') {
            i++;
            ipAddr[j] = '\0';
            portNow = true; 
            j = 0;
        }
        if (portNow == false)
            ipAddr[j++] = addrToParse[i++];
        else 
            portNum[j++] = addrToParse[i++];
    }
    portNum[j] = '\0';

    return portNow;
}

/* goes through the list of services, creats a listener to accept new clients
* for each service in the list */
void initServiceListeners(struct event_base *eBase, service *servList) {
    int portno;
    struct sockaddr_in serv_addr;
    struct in_addr *inp;
    inp = (struct in_addr *) malloc (sizeof(struct in_addr));

    while (servList != NULL) {
        char ipAddr[16], portNum[6];
        bool portNow = false;
        int i = 0; 
        int j = 0; 
        if (!parseAddress(servList->listen, ipAddr, portNum))
            fprintf(stderr, "Bad address unable listen for clients for service %s\n", servList->name);
        else {
            portno = atoi(portNum);
            i = inet_aton(ipAddr, inp); 
            memset(&serv_addr,0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = (*inp).s_addr; 
            serv_addr.sin_port = htons(portno); 
            servList->listener = evconnlistener_new_bind(eBase, clientConnectCB, servList, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
            if (!servList->listener)
                printf("Couldn't create Listener\n");
        }
        servList = servList->next;
    }
}


