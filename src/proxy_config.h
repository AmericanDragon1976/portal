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
service* parse_config_file (char *name);
bool parse_address(char *addrToParse, char *ip_addr, char* port_num);
#endif