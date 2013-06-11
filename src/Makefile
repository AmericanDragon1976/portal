LD_LIBRARY_PATH="/Users/edward/workspace/libevent-2.0.21-stable/" "/usr/local/Cellar/openssl"

proxy: proxy.o proxy_config.o proxy_events.o proxy_structures.o
	gcc -o proxy proxy.o proxy_config.o proxy_events.o proxy_structures.o  -lssl -lcrypto 

proxy.o: proxy.c proxy.h proxy_config.h proxy_events.h proxy_structures.h
	gcc -c proxy.c 

proxy_config.o: proxy_config.c proxy.h proxy_config.h proxy_events.h 
	gcc -c proxy_config.c 

proxy_events.o: proxy_events.c proxy.h proxy_config.h proxy_events.h 
	gcc -c proxy_events.c 

proxy_structures.o: proxy_structures.c proxy.h proxy_config.h proxy_events.h proxy_structures.h
	gcc -c proxy_structures.c

run: proxy
	./proxy