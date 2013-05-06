LD_LIBRARY_PATH="/Users/edward/workspace/libevent-2.0.21-stable/" "/usr/local/Cellar/openssl"

portal-proxy: portal-proxy.o portal-proxy.h proxy.o proxy.h setup.o setup.h 
	../deps/libevent-2.0.21-stable/libtool --mode=link gcc -o portal-proxy portal-proxy.o proxy.o setup.o ../deps/libevent-2.0.21-stable/libevent.la ../deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

portal-proxy.o: portal-proxy.c portal-proxy.h
	gcc -c portal-proxy.c

proxy.o: proxy.c proxy.h
	gcc -c proxy.c 

setup.o: setup.c setup.h
	gcc -c setup.c 

run: portal-proxy
	./portal-proxy

TESTONE: testOne.o 
	deps/libevent-2.0.21-stable/libtool --mode=link gcc -o TESTONE testOne.o  deps/libevent-2.0.21-stable/libevent.la deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

testOne.o: testOne.c 
	gcc -c testOne.c 

runTESTONE: TESTONE
	./TESTONE

Mock: mockMonitor.o
	deps/libevent-2.0.21-stable/libtool --mode=link gcc -o Mock mockMonitor.o  deps/libevent-2.0.21-stable/libevent.la deps/libevent-2.0.21-stable/libevent_openssl.la -lssl -lcrypto 

mockMonitor.o: mockMonitor.c
	gcc -c mockMonitor.c

runMockOne: Mock
	./Mock 4044