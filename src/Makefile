LD_LIBRARY_PATH="/Users/edward/workspace/libevent-2.0.21-stable/" "/usr/local/Cellar/openssl"

portal/portal: portal/portal-proxy.o portal/proxy.o portal/setup.o 
	../deps/libevent-2.0.21-stable/libtool --mode=link gcc -o portal/portal portal/portal-proxy.o portal/proxy.o portal/setup.o ../deps/libevent-2.0.21-stable/libevent.la ../deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

portal/portal-proxy.o: portal/portal-proxy.c portal/portal-proxy.h portal/setup.h portal/proxy.h
	gcc -c portal/portal-proxy.c

portal/proxy.o: portal/proxy.c portal/setup.h portal/proxy.h portal/portal-proxy.h
	gcc -c portal/proxy.c 

portal/setup.o: portal/setup.c portal/proxy.h portal/setup.h portal/portal-proxy.h
	gcc -c portal/setup.c 

run: portal
	./portal/portal
