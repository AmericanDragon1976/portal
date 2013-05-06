#include "portal-proxy.h"
#include "setup.h"

/* called by toEvent every time it times out. They body is commented out
because its current use is for testing some aspects and it may be removed 
from the final product. */
static void timeoutCB(evutil_socket_t fd, short what, void *arg) { 
/*  service *test = (service *) arg;
    bufferevent_write(test->b_monitor, test->name, sizeof(test->name));
    printf("timeoutCB called\n");
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

/* checking flags set Currently only -C flag is used but others could be 
added later by adding additional if statements before checking if flags
have been set at the end of the for loop. */
    for (i = 1; i < argc - 1; i++) { 
        if (!strcmp(argv[i], "-C")) {
            cFlag = true;
            flagsSet = true;                        // TODO: turn this section into a function 
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
toEvent = event_new(base, -1, EV_PERSIST, timeoutCB, serviceList);
event_add(toEvent, &five_seconds);

// create listener to handle proper shutdown in case of interrupt signal
    signal_event = evsignal_new(base, SIGINT, signalCB, (void *) base);
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }

    event_base_dispatch(base);
    freeAllListeners(serviceList);
    freeAllServiceNodes(serviceList);
    event_base_free(base);
}
