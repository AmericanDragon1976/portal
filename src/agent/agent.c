#include "portal-agent.h" 
#include "agentSetup.h"
#include "agent.h"

/*
 * Called when a monitor connects. Sets up an event buffer for that monitor and
 * adds it to the event base. 
 */
void 
monitor_connect_cb(struct evconnlistener *listener, evutil_socket_t fd, 
                   struct sockaddr *address, int socklen, void *ctx)
{
	list_heads			*lists = (list_heads *) ctx;
	buffer_list			*temp_buff_list = (buffer_list *) malloc(sizeof(buffer_list));
	struct event_base   *base = evconnlistener_get_base(listener);
	struct bufferevent 	*temp_bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);

	bufferevent_setcb(temp_buff_list->bev, read_cb, NULL, event_cb, lists);
	bufferevent_enable(temp_buff_list->bev, EV_READ|EV_WRITE);
	temp_buff_list->next = lists->b_list;
	lists->b_list = &temp_buff_list;
}

/*
 *
 */
void
read_cb(struct bufferevent *bev, void *heads){}

/*
 *
 */
void
event_cb(struct bufferevent *bev, short what, void *ctx){}