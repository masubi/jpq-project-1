#Makefile: if you use gcc, then change g++ to gcc
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

default: clean graphexe

graphexe : main.cpp graphexe.cpp graphexe.h 
	$(CC) $(LFLAGS) graphexe.cpp main.cpp -o graphexe
	chmod 700 graphexe

test: test.cpp
	$(CC) $(LFLAGS) test.cpp -o test
	chmod 700 test
	
clean:
	rm -f graphexe
	rm -f test

