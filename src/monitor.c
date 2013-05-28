#include "monitor.h"
#include "monitor_config.h"
#include "monitor_events.h"

/* 
 * Prints to screen the proper syntax for running the program, then exits.
 */
void 
usage()
{
    printf("Usage is as follows: \n");
    printf("    monitor space seperated flags /path/to/config/file\n");
    printf("Example: \n");
    printf("    monitor -C ../../deps/config.txt\n");
    exit(0);
}

/* 
 * Verifys the command line areguament for the monitor, returns ture if correct 
 * argumets are supplied and false otherwise. 
 */
bool 
validate_args(int argc, char **argv) 
{
    if (argc < 3)
        return (false);

    if (strcmp(argv[1], "-C") != 0) // if other flags added or effect of -C flag changes alter here. 
        return (false);

    return (true);
}

/*
 * main
 */
 int
main (int argc, char **argv) 
{
	struct event_base *event_loop = NULL;

    if (!validate_args(argc, argv)) 
    	usage();

   = parse_config_file(argv[argc - 1]);	//TODO: set return type and create a variable to catch the returning value

  prep_services();  	// TODO: set parameters, will pass in what parse_config_file() returns
  init_proxy_listeners();
  event_loop = event_base_new();  //  TODO: parameters to be set
    	//TODO: main
}