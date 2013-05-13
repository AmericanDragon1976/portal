#include "portal-monitor.h"
#include "monitorSetup.h"
#include "monitor.h"

/* Verifys the command line areguament for the monitor, returns ture if correct 
* argumets are supplied and false otherwise. */
bool verifyComndArgs(int argc, char **argv) {
    if (argc < 2)
        return false;

    return true;
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
        fprintf(stderr, "Unable to open config file. check file name and path!\n");
        exit(0);
    }

    fseek(filePtr, 0, SEEK_END);
    fileLen = ftell(filePtr);
    fclose(filePtr);

    return fileLen;
}

/*  Read a file, Recive name of file with length of <= 100, and int var (len) by referance
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

/* recives a service pointer to buffer containing config file
* returns pointer that is the head of a list of services. */
moniServ* parseConfigFile(char *buff, int len) { /* TODO: parse the file and create linked list */
    moniServ *listHead = newNullMoniServNode();
    moniServ *currentRecord = listHead;
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
        for (; buff[i++] != 'y';);                  // find end of identifier "proxy"
        i++;                                        // advance beyond white space
        j = 0;
        while (buff[i] != '\n')
            currentRecord->proxyAddr[j++] = buff[i++]; // read proxy address
        currentRecord->proxyAddr[j] = '\0';
        for (;buff[i++] != 't';);                   // find end of identifier "agent"
        i++;                                        // advance beyond white space
        j = 0;
        while (i < len && buff[i] != '\n')
            currentRecord->agentAddr[j++] = buff[i++];// read agent address
        currentRecord->agentAddr[j] = '\0';

        if (i < len){
            currentRecord->next = newNullMoniServNode();
            currentRecord = currentRecord->next;
            for (; buff[i++] != 's';);              // advance to next record
            i--;
        }
    }
}

/* creates a new instance of a moniServ struct with all pointers inicilized to NULL */
moniServ* newNullMoniServNode() {
    moniServ *nwMoniServ = (moniServ *) malloc(sizeof(moniServ));
    nwMoniServ->listener = NULL;
    nwMoniServ->proxyList = NULL;
    nwMoniServ->next = NULL;

    return nwMoniServ;
}
/* creates a new instance of moniServe struct with pointers set to the passed in values */
moniServ* newMoniServNode(struct evconnlistener* lstnr, proxy* prxyLst, moniServ* service) {
    moniServ *nwMoniServ = (moniServ *) malloc(sizeof(moniServ));
    nwMoniServ->listener = lstnr;
    nwMoniServ->proxyList = prxyLst;
    nwMoniServ->next = service;

    return nwMoniServ;
}
        // connect to the agent(s) for the serivce(s) being monitored 
        // get current address for service(s) being monitored from the agent(s)
        // establish listener(s) for proxy agent(s) 