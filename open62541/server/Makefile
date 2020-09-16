.DEFAULT_GOAL := default

.PHONY = clean

increment: server.c
	@echo "\e[100mINCREMENET SENZA OTTIMIZZAZIONE\033[0m"
	gcc -std=c99 -DUA_ARCHITECTURE_POSIX -D ENABLE_VARIABLE_INCREMET -I../lib ../lib/open62541.c ../lib/measures_lib.c  server.c -o server -lm


default: server.c
	@echo "\e[41mNO INCREMENT SENZA OTTIMIZZAZIONE\033[0m"
	gcc -std=c99 -DUA_ARCHITECTURE_POSIX -I../lib ../lib/open62541.c ../lib/measures_lib.c server.c -o server -lm

incremento3: server.c
	@echo "\e[42mINCREMENET OTTIMIZZAZIONE\033[0m"
	gcc -std=c99 -O3 -s -DUA_ARCHITECTURE_POSIX -D ENABLE_VARIABLE_INCREMET -I../lib ../lib/open62541.c ../lib/measures_lib.c  server.c -o server -lm


normo3: server.c
	@echo "\e[104mNO INCREMENT OTTIMIZZAZIONE\033[0m"
	gcc -std=c99 -O3 -s -DUA_ARCHITECTURE_POSIX -I../lib ../lib/open62541.c ../lib/measures_lib.c server.c -o server -lm


clean:
	rm server