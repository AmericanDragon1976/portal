LD_LIBRARY_PATH="/Users/edward/workspace/libevent-2.0.21-stable/" "/usr/local/Cellar/openssl"

proxy/portal: proxy/portal-proxy.o proxy/proxy.o proxy/proxySetup.o 
	../deps/libevent-2.0.21-stable/libtool --mode=link gcc -o proxy/portal proxy/portal-proxy.o proxy/proxy.o proxy/proxySetup.o ../deps/libevent-2.0.21-stable/libevent.la ../deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

portal-proxy.o: proxy/portal-proxy.c proxy/portal-proxy.h proxy/proxySetup.h proxy/proxy.h
	gcc -c proxy/portal-proxy.c 

proxy.o: proxy/proxy.c proxy/proxySetup.h proxy/proxy.h proxy/portal-proxy.h
	gcc -c proxy/proxy.c 

proxySetup.o: proxy/proxySetup.c proxy/proxy.h proxy/proxySetup.h proxy/portal-proxy.h
	gcc -c proxy/proxySetup.c 

run: portal
	./proxy/portal

monitor/monitor: monitor/portal-monitor.o monitor/monitorSetup.o monitor/monitor.o
	../deps/libevent-2.0.21-stable/libtool --mode=link gcc -o monitor/monitor monitor/portal-monitor.o monitor/monitor.o monitor/monitorSetup.o ../deps/libevent-2.0.21-stable/libevent.la ../deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto

portal-monitor.o: monitor/portal-monitor.c monitor/portal-monitor.h monitor/monitorSetup.h monitor/monitor.h
	gcc -c monitor/portal-monitor.c

monitor: monitor/monitor.c monitor/monitor.h monitor/monitorSetup.h monitor/portal-monitor.h
	gcc -c monitor/monitor.c

monitorSetup: monitor/monitorSetup.c monitor/monitorSetup.h monitor/monitor.h monitor/portal-monitor.h
	gcc -c monitor/monitor 