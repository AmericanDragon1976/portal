#ifndef AGENT_CONFIG_H
#define AGENT_CONFIG_H

void print_hook_list(hook_list *list);
int get_config_file_len(char *name);
char* read_file(char *name, int len);
void parse_config_file(char *name, service svc_list[]);

#endif