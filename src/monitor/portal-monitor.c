#include "portal-monitor.h"
#include "portal-monitor.h"
#include "monitor.h"
#include "monitorSetup.h"

int main (int argc, char **argv) {
    moniServ *serviceList = NULL;
    char fileName[100];
    char *fileBuffer = NULL, **cmdArgs = NULL;
    FILE *filePtr = NULL;
    int fileSize;
    struct event_base *base = NULL;

    cmdArgs = argv;
    if (verifyComndArgs(argc, cmdArgs)) {
        strcpy(fileName, cmdArgs[1]);
        fileSize = getConfigFileLen(fileName);
        fileBuffer = readFile(fileName, fileSize);
        serviceList = parseConfigFile(fileBuffer, fileSize);
    }
    else {
         /* use defaults */ ;
    }

    base = event_base_new();
    contactAgents(base, serviceList);

        // connect to the agent(s) for the serivce(s) being monitored 
        // establish listener(s) for proxy agent(s) 
        // get current address for service(s) being monitored from the agent(s)
        // establish listeners for proxys 
    
    // Monitoring activity 
        // respond to proxy(s) that are requesting inicial data,
        // maintain a list of services, who needs updates on them, and their agents
        // in a cycle, (no more frequently that once every x seconds), request each service's agent test its service and update status
        // broadcast to proxy if address needs changed
}