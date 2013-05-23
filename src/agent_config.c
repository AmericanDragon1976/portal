#include "agent.h" 
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
    FILE    *file_ptr;

    file_ptr = fopen(name, "r");

    if (file_ptr == NULL) {
        fprintf(stderr, "Unable to open config file. check file name and path!\n");
        exit(0);
    }

    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr);
    fclose(file_ptr);

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
 * Recives a service pointer to the buffer containing config file
 * Returns a service list, each service has a name and a list of 
 * commands it can use to test the service. 
 */
serv_lst* 
parse_config_file(char *buff, int len) 
{
    serv_lst    *head = NULL; 
    serv_lst    *curr_node = NULL;
    char        text[file_nm_len] = {};
    char        hook_nm[hook_len];
    char        hook_path[file_nm_len];
    int         i, j;

    if(buff == NULL || len < 1){
        fprintf(stderr, "Errer reading config file!!\n");
        free(head);
        exit(0);
    }

    j = 0; 
 
    for (i = 0; buff[i] == ' '; i++)                  // advance beyond any leading whitespace
        ;

    bzero(&text, file_nm_len);

    while (buff[i++] != ' ')
        text[j++] = buff[i - 1];            // read in word service
    text[j] = '\0';

    if (strcmp(text, "service") != 0) { 
        fprintf(stderr, "fatal error, bad config file.\n");
        exit(0);  
    }

    while (i < len) {
        curr_node = new_null_serv_lst();
        j = 0;
       bzero(&text, file_nm_len);

        while (buff[i++] != '\n')
            text[j++] = buff[i - 1];

        text[j] = '\0';
        strcpy(curr_node->name, text);
        bzero(&text, file_nm_len);
        j = 0; 

        for (; buff[i] == ' '; i++)
            ;

        while(buff[i++] != ' ')
            text[j++] = buff[i - 1];

            text[j] = '\0';

        while (strcmp(text, "service") != 0){
            hook_path_pair *temp_hook_lst = new_null_hook_path_pair();

            strcpy(temp_hook_lst->hook, text); 
            j = 0;
           bzero(&text, file_nm_len);

            while(i < len && buff[i++] != '\n'){
                text[j++] = buff[i - 1];
            } 

            text[j] = '\0';
            strcpy(temp_hook_lst->path, text); 
            j = 0;
            bzero(&text, file_nm_len);

            temp_hook_lst->next = curr_node->cmd_lst;
            curr_node->cmd_lst = temp_hook_lst;

            if (i < len){
                for (; buff[i] == ' '; i++)
                    ;

                while(buff[i++] != ' ')
                    text[j++] = buff[i -1];

                text[j] = '\0';
            } else {
                break;
            }
        }
        curr_node->next = head;
        head = curr_node;
    }
    return (head);
}
