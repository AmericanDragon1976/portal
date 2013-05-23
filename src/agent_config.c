#incslude "agent.h" 
#include "agent_config.h"
#include "agent_events.h"

/* 
 * Returns the length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array once it is read in. 
 */
int 
get_config_file_len(char *name)
{
    int     file_len = 0;
    char    *buffer;
    FILE    *file_pointer;

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
 * Read a file, Recive name of file and length. 
 * Return pointer to char array (buffer) holding contents of the file. 
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
 * Recives a service pointer to the buffer containing config file
 * Returns a service list, each service has a name and a list of 
 * commands it can use to test the service. 
 */
svc_list* 
parse_config_file(char *buffer, int len) 
{
    svc_list    *head = NULL; 
    svc_list    *current_node = NULL;
    char        text[file_name_len] = {};
    char        hook_name[hook_len];
    char        hook_path[file_name_len];
    int         i, j;

    if(buffer == NULL || len < 1){
        fprintf(stderr, "Errer reading config file!!\n");
        free(head);
        exit(0);
    }

    j = 0; 
 
    for (i = 0; buffer[i] == ' '; i++)                  // advance beyond any leading whitespace
        ;

    bzero(&text, file_name_len);

    while (buffer[i++] != ' ')
        text[j++] = buffer[i - 1];            // read in word service
    text[j] = '\0';

    if (strcmp(text, "service") != 0) { 
        fprintf(stderr, "fatal error, bad config file.\n");
        exit(0);  
    }

    while (i < len) {
        current_node = new_null_svc_list();
        j = 0;
       bzero(&text, file_name_len);

        while (buffer[i++] != '\n')
            text[j++] = buffer[i - 1];

        text[j] = '\0';
        strcpy(current_node->name, text);
        bzero(&text, file_name_len);
        j = 0; 

        for (; buffer[i] == ' '; i++)
            ;

        while(buffer[i++] != ' ')
            text[j++] = buffer[i - 1];

            text[j] = '\0';

        while (strcmp(text, "service") != 0){
            hook_path_pair *temp_hook_list = new_null_hook_path_pair();

            strcpy(temp_hook_list->hook, text); 
            j = 0;
           bzero(&text, file_name_len);

            while(i < len && buffer[i++] != '\n'){
                text[j++] = buffer[i - 1];
            } 

            text[j] = '\0';
            strcpy(temp_hook_list->path, text); 
            j = 0;
            bzero(&text, file_name_len);

            temp_hook_list->next = current_node->command_lst;
            current_node->command_lst = temp_hook_list;

            if (i < len){
                for (; buffer[i] == ' '; i++)
                    ;

                while(buffer[i++] != ' ')
                    text[j++] = buffer[i -1];

                text[j] = '\0';
            } else {
                break;
            }
        }
        current_node->next = head;
        head = current_node;
    }
    return (head);
}
