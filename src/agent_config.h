#ifndef AGENT_CONFIG_H
#define AGENT_CONFIG_H

int 
get_config_file_len(char *name);

char* 
read_file(char *name, int len);

svc_list* 
parse_config_file(char *buff, int len);

#endif