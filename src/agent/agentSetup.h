#ifndef AGENTSETUP_H
#define AGENTSETUP_H

bool 
verify_comnd_ln_args(int argc, char **argv);

int 
get_config_file_len(char *name);

char* 
read_file(char *name, int len);

serv_lst* 
parse_config_file(char *buff, int len);

serv_lst*
new_serv_lst(serv_lst *nxt, hook_path_pair *cmd_lst_head);

serv_lst*
new_null_serv_lst();

hook_path_pair*
new_null_hook_path_pair();

hook_path_pair*
new_hook_path_pair(hook_path_pair *nxt);

void
listen_for_monitors(struct event_base *base, struct evconnlistener *listner, list_heads *heads);

void 
usage(); 
#endif