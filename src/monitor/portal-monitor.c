#include <portal-monitor.h>

int main (int argc, char **argv) {
    // Inicilize settings 
        // read a config file/ if no config is present use defaults. 
        // connect to the agent(s) for the serivce(s) being monitored 
        // get current address for service(s) being monitored from the agent(s)
        // establish listener(s) for proxy agent(s) 
    // Monitoring activity 
        // respond to proxy(s) that are requesting inicial data,
        // maintain a list of services, who needs updates on them, and their agents
        // in a cycle, (no more frequently that once every x seconds), request each service's agent test its service and update status
        // broadcast to proxy if address needs changed
}