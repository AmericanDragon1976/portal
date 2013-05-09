#include "portal-monitor.h"
#include "portal-monitor.h"

int main (int argc, char **argv) {
    moniServ *serviceList = NULL;
    char fileName[100];
    char *fileBuffer = NULL, **cmdArgs = NULL;
    FILE *filePtr = NULL;
    int filesize;

    cmdArgs = argv;
    if (!verifyComndArgs(argc, cmdArgs)) 
         /* use defaults */ ;
    else {
        strcpy(filename, cmdArgs[1])
        filesize = getConfigFileLen(filename);
        fileBuffer = readFile(filename, filesize);
        serviceList = parseConfigFile(fileBuffer, fileSize);
    }

        // connect to the agent(s) for the serivce(s) being monitored 
        // get current address for service(s) being monitored from the agent(s)
        // establish listener(s) for proxy agent(s) 
    
    // Monitoring activity 
        // respond to proxy(s) that are requesting inicial data,
        // maintain a list of services, who needs updates on them, and their agents
        // in a cycle, (no more frequently that once every x seconds), request each service's agent test its service and update status
        // broadcast to proxy if address needs changed
}