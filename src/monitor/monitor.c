#include "portal-monitor.h"
#include "monitor.h"
#include "monitorSetup.h"

/* creates a new instance of the struct proxy with all pointers inicilized to NULL*/
proxy* newNullProxy (){
	proxy *nwProxy = (proxy *) malloc(sizeof(proxy));
	nwProxy->bProxy = NULL;
	nwProxy->next = NULL;

	return nwProxy;
}
/* creates a new instance of the struct proxy with pointers set to passed in value*/
proxy* newProxy (struct bufferevent* bevProxy, proxy* nxt){
	proxy *nwProxy = (proxy *) malloc(sizeof(proxy));
	nwProxy->bProxy = bevProxy;
	nwProxy->next = nxt;

	return nwProxy;
}

