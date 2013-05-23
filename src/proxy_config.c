#include "proxy.h"
#include "proxy_config.h"
#include "proxy_events.h"

/* 
 * Recives the name and path to athe config file. Returns an int congtining the
 * length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array after it is read in. 
 */
int 
get_config_file_len(char *name) 
{
    int     file_len = 0;
    char    *buffer;
    FILE    *file_pointer;

    file_pointer = fopen(name, "r");
    if (file_pointer == NULL) {
        fprintf(stderr, "Unable to open config file. chech file name and path!\n");
        exit(0);
    }

    fseek(file_pointer, 0, SEEK_END);
    file_len = ftell(file_pointer);
    fclose(file_pointer);

    return (file_len);
}

/* 
 * Recives the name a path to the config file, and a int containing the length 
 * of the file. Returns pointer to a char array holding contents of the file.  
 */
char* 
read_file(char *name, int len)
{
    char    *buffer = (char *) malloc(sizeof(char) * (len));
    FILE    *file_pointer;
    size_t  result;

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
 * Recives a char pointer to the buffer containing the config file text. 
 * Returns a pointer to the head of a linked list of services. 
 */
service* 
parse_config_file(char *name)
{
    service     *list_head = new_null_service_node();
    service     *current_record = list_head;
    char        service_start_identifier[] = "service";
    int 		len = get_config_file_len(name);
    char 		*buffer = read_file(name, len);

    int j = 0;
    int i = 0;

    while (i < len){
        for (j = 0; j < sizeof(service_start_identifier) - 1; j++)  // read Identifier "service"
            service_start_identifier[j] =  buffer[i++];

        i++;                                                         // advance past white space

        if (strcmp(service_start_identifier, "service")){           // returns 0 (false) only if they are equal
            fprintf(stderr, "Config file Corrupted. \n");
            exit(0);
        }

        j = 0;

        while (buffer[i] != '\n') 
            current_record->name[j++] = buffer[i++];   // read service name

        current_record->name[j] = '\0';
        i++;                                        // disgard \n

        for (; buffer[i++] != 'n';);                  // find end of identifier "listen"

        i++;                                        // advance beyond white space
        j = 0;

        while (buffer[i] != '\n')
            current_record->listen[j++] = buffer[i++]; // read listen address

        current_record->listen[j] = '\0';

        for (;buffer[i++] != 'r';)                    // find end of identifier "monitor"
            ;                   
        i++;                                        // advance beyond white space
        j = 0;

        while (i < len && buffer[i] != '\n')
            current_record->monitor[j++] = buffer[i++];// read monitor address

        current_record->monitor[j] = '\0';

        if (i < len){
            current_record->next = new_null_service_node();

            current_record = current_record->next;
            for (; buffer[i++] != 's';)              // advance to next record
                ;

            i--;
        }
    }
    free(buffer);
    return list_head;
}

/* 
 * Takes an adrress in the form a.b.c.d:port_numberber and parses it storing the ip
 * address and port number in the approiate char arrays, address_to_parse[22], ip_address[16] 
 * and port_number[6], returns true if successful otherwise returns false 
 */
bool 
parse_address(char *address_to_parse, char *ip_address, char* port_number) 
{
    int     i, j;
    bool    port_now = false;

    j = 0;

    if ( address_to_parse == NULL)
        return port_now;

    for (i = 0; i < complete_address_len; ){
        if (address_to_parse[i] == ':') {
            i++;
            ip_address[j] = '\0';
            port_now = true; 
            j = 0;
        }

        if (port_now == false)
            ip_address[j++] = address_to_parse[i++];
        else 
            port_number[j++] = address_to_parse[i++];
    }

    port_number[j] = '\0';

    return port_now;
}
