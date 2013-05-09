#include "portal-monitor.h"
#include "monitorSetup.h"

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
service* parseConfigFile(char *buff, int len) { /* TODO: parse the file and create linked list */}

        // connect to the agent(s) for the serivce(s) being monitored 
        // get current address for service(s) being monitored from the agent(s)
        // establish listener(s) for proxy agent(s) 