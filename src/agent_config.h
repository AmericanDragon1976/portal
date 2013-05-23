#ifndef AGENT_CONFIG_H
#define AGENT_CONFIG_H

int 
get_config_file_len(char *name);

char* 
read_file(char *name, int len);

serv_lst* 
parse_config_file(char *buff, int len);

#endif