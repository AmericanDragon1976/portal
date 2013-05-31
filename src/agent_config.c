#include "agent_structures.h"
#include "agent.h" 
#include "agent_config.h"
#include "agent_events.h"

/*
 * Takes a pointer to a linked list of agent services and prints the information contained in 
 * the linked list to stdout.  
 */
void print_hook_list(hook_list *list)
{
    hook_path_node      *current_node = list->head;
    hook_path_pair      *temp_pair = NULL;
    
    while (current_node != NULL){ 
        temp_pair = current_node->pair;
        printf("command: %s, Path: %s\n", temp_pair->hook, temp_pair->path);
        current_node = current_node->next;
    }
}

/* 
 * Returns the length of the config file. There is a good probility that 
 * the file will contain 0's and so strlen() can not be used to determine 
 * the length of the char array once it is read in. 
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
 * Read a file, Recive name of file and length. 
 * Return pointer to char array (buffer) holding contents of the file. 
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
 * Recives a service pointer to the buffer containing config file
 * Returns a service list, each service has a name and a list of 
 * commands it can use to test the service. 
 */ 
void 
parse_config_file(char *name, service svc_list[])
{
    int         current_svc = 0;
    char        service_start_identifier[] = "service";
    int         len = get_config_file_len(name);
    char        *buffer = read_file(name, len);
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

        j = 0;
       bzero(&text, file_name_len);

        while (buffer[i++] != '\n')
            text[j++] = buffer[i - 1];

        text[j] = '\0';
        strcpy(svc_list[current_svc].name, text);
        bzero(&text, file_name_len);
        j = 0; 

        for (; buffer[i] == ' '; i++)
            ;

        while(buffer[i++] != ' ')
            text[j++] = buffer[i - 1];

            text[j] = '\0';

        while (strcmp(text, "service") != 0){
            hook_path_pair *temp_hook_list_pair = new_null_hook_path_pair();

            strcpy(temp_hook_list_pair->hook, text); 
            j = 0;
           bzero(&text, file_name_len);

            while(i < len && buffer[i++] != '\n'){
                text[j++] = buffer[i - 1];
            } 

            text[j] = '\0';
            strcpy(temp_hook_list_pair->path, text); 
            j = 0;
            bzero(&text, file_name_len);

            hook_path_node      *temp_node = (hook_path_node *) malloc(sizeof(hook_path_node));

            temp_node->pair = temp_hook_list_pair;
            temp_node->next = svc_list[current_svc].list_of_hooks->head;
            svc_list[current_svc].list_of_hooks->head = temp_node;

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
        current_svc++;
    }
    free(buffer);
    return (head);
}
