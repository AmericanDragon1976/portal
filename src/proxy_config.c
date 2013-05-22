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
    FILE    *file_ptr;

    file_ptr = fopen(name, "r");
    if (file_ptr == NULL) {
        fprintf(stderr, "Unable to open config file. chech file name and path!\n");
        exit(0);
    }

    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr);
    fclose(file_ptr);

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
    FILE    *file_ptr;
    size_t  result;

    file_ptr = fopen(name, "r");
    if (file_ptr == NULL) {
        fprintf(stderr, "Unable to open file, check file name and path!\n");
        exit(0);
    }

    if (buffer == NULL) {
        fprintf(stderr, "Memory Error, creation of File Buffer Failed!\n");
        exit(0);
    }
    result = fread(buffer, 1, len, file_ptr);
    if (result != len) {
        fprintf(stderr, "Error reading file.\n");
        exit(0);
    }
    fclose(file_ptr);
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
    char        serIdent[] = "service";
    int 		len = get_config_file_len(name);
    char 		*buffer = read_file(name, len);

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
            current_record->name[j++] = buff[i++];   // read service name

        current_record->name[j] = '\0';
        i++;                                        // disgard \n

        for (; buff[i++] != 'n';);                  // find end of identifier "listen"

        i++;                                        // advance beyond white space
        j = 0;

        while (buff[i] != '\n')
            current_record->listen[j++] = buff[i++]; // read listen address

        current_record->listen[j] = '\0';

        for (;buff[i++] != 'r';)                    // find end of identifier "monitor"
            ;                   
        i++;                                        // advance beyond white space
        j = 0;

        while (i < len && buff[i] != '\n')
            current_record->monitor[j++] = buff[i++];// read monitor address

        current_record->monitor[j] = '\0';

        if (i < len){
            current_record->next = new_null_service_node();

            current_record = current_record->next;
            for (; buff[i++] != 's';)              // advance to next record
                ;

            i--;
        }
    }
    free(buffer);
    return list_head;
}

/* 
 * Takes an adrress in the form a.b.c.d:port_number and parses it storing the ip
 * address and port number in the approiate char arrays, addrToParse[22], ip_addr[16] 
 * and port_num[6], returns true if successful otherwise returns false 
 */
bool 
parse_address(char *addrToParse, char *ip_addr, char* port_num) 
{
    int     i, j;
    bool    port_now = false;

    j = 0;

    if ( addrToParse == NULL)
        return port_now;

    for (i = 0; i < comp_add_len; ){
        if (addrToParse[i] == ':') {
            i++;
            ip_addr[j] = '\0';
            port_now = true; 
            j = 0;
        }

        if (port_now == false)
            ip_addr[j++] = addrToParse[i++];
        else 
            port_num[j++] = addrToParse[i++];
    }

    port_num[j] = '\0';

    return port_now;
}
