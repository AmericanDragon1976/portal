LD_LIBRARY_PATH="/Users/edward/workspace/libevent-2.0.21-stable/" "/usr/local/Cellar/openssl"

proxy: proxy.o proxy_config.o proxy_events.o 
	../../libevent-2.0.21-stable/libtool --mode=link gcc -o proxy proxy.o proxy_config.o proxy_events.o ../../libevent-2.0.21-stable/libevent.la ../../libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

proxy.o: proxy.c proxy.h proxy_config.h proxy_events.h 
	gcc -c proxy.c 

proxy_config.o: proxy.c proxy.h proxy_config.h proxy_events.h 
	gcc -c proxy_config.c 

proxy_events.o: proxy_events.c proxy.h proxy_config.h proxy_events.h 
	gcc -c proxy_events.c 

run: proxy
	./proxy



monitor: monitor.o monitor_config.o monitor_events.o
	../../libevent-2.0.21-stable/libtool --mode=link gcc -o monitor monitor.o monitor_config.o monitor_events.o ../../libevent-2.0.21-stable/libevent.la ../../libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto

monitor.o: monitor.c monitor.h monitor_config.h monitor_events.h
	gcc -c monitor.c

monitor_config.o: monitor_config.c monitor.h monitor_config.h monitor_events.h
	gcc -c monitor_config.c

monitor_events.o: monitor_events.c monitor.h monitor_config.h monitor_events.h
	gcc -c monitor_events.c



agent: agent.o agent_config.o agent_events.o
	../../libevent-2.0.21-stable/libtool --mode=link gcc -o agent agent.o agent_config.o agent_events.o ../../libevent-2.0.21-stable/libevent.la ../../libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto

agent.o: agent.c agent.h agent_config.h agent_events.h
	gcc -c agent.c

agent_config.o: agent_config.c agent.h agent_config.h agent_events.h
	gcc -c agent_config.c

agent_events.o: agent_events.c agent.h agent_config.h agent_events.h
	gcc -c agent_events.c 