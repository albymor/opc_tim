.DEFAULT_GOAL := default

.PHONY = clean

default: client.c
	@echo "\e[41mSENZA OTTIMIZZAZIONE\033[0m"
	gcc -std=c99 -DUA_ARCHITECTURE_POSIX -I../lib ../lib/open62541.c ../lib/measures_lib.c client.c -o client -lm

o3: client.c
	@echo "\e[42mOTTIMIZZAZIONE\033[0m"
	gcc -std=c99 -O3 -s -DUA_ARCHITECTURE_POSIX -I../lib ../lib/open62541.c ../lib/measures_lib.c client.c -o client -lm


clean:
	rm client
