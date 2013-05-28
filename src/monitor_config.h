#ifndef MONITOR_CONFIG_H
#define MONITOR_CONFIG_H

int get_config_file_len(char *name);
char* read_file(char *name, int len);
void parse_config_file(char *name); 			//TODO: will return something when finished. 

#endif