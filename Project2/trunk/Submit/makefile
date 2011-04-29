OBJS=main.o 
SRC=$(OBJS:.o=.cpp)
EXE=eaglefeed
CC=g++
DEBUG=-g
CFLAG=-Wall -c $(DEBUG)
LFLAG=-Wall $(DEBUG) -lpthread

$(EXE): clean $(OBJS) 
	$(CC) -o $@ $(OBJS) $(LFLAG)
	chmod 744 $(EXE)
	./$(EXE) 5 8 4

test:
	$(CC) -o $@ test.cpp $(LFLAG)

clean:
	rm -f $(OBJS) $(EXE) 
	
.cpp.o:
	$(CC) $(CFLAG) $<

