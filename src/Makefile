LD_LIBRARY_PATH="/Users/edward/workspace/libevent-2.0.21-stable/" "/usr/local/Cellar/openssl"

portal/portal: portal/portal-proxy.o portal/proxy.o portal/proxySetup.o 
	../deps/libevent-2.0.21-stable/libtool --mode=link gcc -o portal/portal portal/portal-proxy.o portal/proxy.o portal/proxySetup.o ../deps/libevent-2.0.21-stable/libevent.la ../deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

portal-proxy.o: portal/portal-proxy.c portal/portal-proxy.h portal/proxySetup.h portal/proxy.h
	gcc -c portal/portal-proxy.c 

proxy.o: portal/proxy.c portal/proxySetup.h portal/proxy.h portal/portal-proxy.h
	gcc -c portal/proxy.c 

proxySetup.o: portal/proxySetup.c portal/proxy.h portal/proxySetup.h portal/portal-proxy.h
	gcc -c portal/proxySetup.c 

run: portal
	./portal/portal
