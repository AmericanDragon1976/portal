#include "portal-proxy.h"
#include "setup.h"
#include "proxy.h"

/* called by toEvent every time it times out. They body is commented out
because its current use is for testing some aspects and it may be removed 
from the final product. */
void timeoutCB(evutil_socket_t fd, short what, void *arg) { 
/*  service *test = (service *) arg;
    bufferevent_write(test->b_monitor, test->name, sizeof(test->name));
    printf("timeoutCB called\n");
    printf("Server up\n");
*/}

int main(int argc, char **argv) {
    int i = 0;
    service *serviceList = NULL;
    char fileName[100];
    FILE *filePointer;
    int fileSize;
    char *fileBuffer, **cmdArgs;
    struct event_base *base;
    struct event *signalEvent;

    /* Currently -C flag is required but has no effect, if  others are
    * added later to change behavior change the verfyComndlnArgs() function 
    * to change what they do*/

    cmdArgs = argv;
    if (!verifyComndLnArgs(argc, cmdArgs))
        usage();

    strcpy(fileName, argv[2]);
    fileSize = getConfigFileLen(fileName);
    fileBuffer = readFile(fileName, fileSize);
    serviceList = parseConfigFile(fileBuffer, fileSize);    
    free(fileBuffer);
 
    base = event_base_new();

    initServices(base, serviceList);
    initServiceListeners(base, serviceList); 

    // This time out event only needed for testing, can be removed in final version
    struct timeval five_seconds = {5, 0};
    struct event *toEvent; 
    toEvent = event_new(base, -1, EV_PERSIST, timeoutCB, serviceList);
    event_add(toEvent, &five_seconds);

    signalEvent = evsignal_new(base, SIGINT, signalCB, (void *) base);
    if (!signalEvent || event_add(signalEvent, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }

    event_base_dispatch(base);
    freeAllListeners(serviceList);
    freeAllServiceNodes(serviceList);
    event_base_free(base);
}
