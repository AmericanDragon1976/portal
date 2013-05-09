LD_LIBRARY_PATH="/Users/edward/workspace/libevent-2.0.21-stable/" "/usr/local/Cellar/openssl"

src/portal/portal: src/portal/portal-proxy.o src/portal/proxy.o src/portal/proxySetup.o 
	deps/libevent-2.0.21-stable/libtool --mode=link gcc -o src/portal/portal src/portal/portal-proxy.o src/portal/proxy.o src/portal/proxySetup.o deps/libevent-2.0.21-stable/libevent.la deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

src/portal/portal-proxy.o: src/portal/portal-proxy.c src/portal/portal-proxy.h src/portal/proxySetup.h src/portal/proxy.h
	gcc -c portal/portal-proxy.c

src/portal/proxy.o: src/portal/proxy.c src/portal/proxySetup.h src/portal/proxy.h src/portal/portal-proxy.h
	gcc -c src/portal/proxy.c 

src/portal/proxySetup.o: src/portal/proxySetup.c src/portal/proxy.h src/portal/proxySetup.h src/portal/portal-proxy.h
	gcc -c src/portal/setup.c 

run: portal
	./src/portal/portal
