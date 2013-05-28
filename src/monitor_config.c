#include "monitor.h"
#include "monitor_config.h"
#include "monitor_events.h"

/*
 * Recives the name of a file including the path to the file and returns the length of that file as an int.
 */
int 
get_config_file_len(char *name)
{
    int     file_len = 0;
    char    *buffer = NULL;
    FILE    *file_pointer = NULL;

    file_pointer = fopen(name, "r");

    if (file_pointer == NULL) {
        fprintf(stderr, "Unable to open config file. check file name and path!\n");
        exit(0);
    }

    fseek(file_pointer, 0, SEEK_END);
    file_len = ftell(file_pointer);
    fclose(file_pointer);

	return (file_len);
}

/*
 * Recives the name of a file including the path to the file and reads the file into a char array and returns the array.
 */
char* 
read_file(char *name, int len)
{
    char    *buffer = (char *) malloc(sizeof(char) * (len));
    FILE    *file_pointer = NULL;
    size_t  result = NULL;

    file_pointer = fopen(name, "r");
    if (file_pointer == NULL) {
        fprintf(stderr, "Unable to open file, check file name and path!\n");
        exit(0);
    }

    if (buffer == NULL) {
        fprintf(stderr, "Memory Error, creation of File Buffer Failed!\n");
        exit(0);
    }
    result = fread(buffer, 1, len, file_pointer);
    if (result != len) {
        fprintf(stderr, "Error reading file.\n");
        exit(0);
    }
    fclose(file_pointer);
    return (buffer);
}

/*
 * Recives the name of a file including the path to the file, reads it in, and parses and organizes the data. Returns a pointer to the structure
 * holding the organized data. 
 */
void 
parse_config_file(char *name)
{
	int 	len = 0;
	char	*config_file = NULL;

	len = get_cofig_file_len(name);
	config_file = read_file(name, len);
	
}