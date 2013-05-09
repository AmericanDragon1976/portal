#ifndef MONITORSETUP_H
#define MONITORSETUP_H

/*                   PRTOTYPES                 */

/* returns the length of the config file. There is a good probility that 
* the file will contain 0's and so strlen() can not be used to determine 
* the length of the char array once it is read in. */
int getConfigFileLen(char *name);

/*  Read a file, Recive name of file with length of <= 100, and int var (len) by referance
* return pointer to c string (buffer) holding contents of the file, int will now contain
* length of the of the buffer. len needs set so that the info is avaliable later. */
char* readFile(char *name, int len);
#endif 