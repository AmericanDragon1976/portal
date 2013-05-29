/* 
 * Containd all the functions used to setup the portal proxy. Verifys that 
 * all needed informatin is included from the command line, flags and the config 
 * file's name and location. Parses the config file, creates the list of services 
 * and requests the starting address to proxy clients to for each service. In 
 * general anything need to prepare to recive client connections. 
 */

#ifndef PROXY_CONFIG_H
#define PROXY_CONFIG_H

int get_config_file_len(char *name);
char* read_file(char *name, int len);
void parse_config_file (char *name, service svc_list[], int list_length);
bool parse_address(char *address_to_parse, char *ip_address, char* port_number);
#endif