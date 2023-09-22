OBJECTS = main.o

EXEC = wipry-lp

BUILDTIMESTAMP = \"`date -u +"%Y-%m-%dT%H:%M:%SZ"`\"
CC = gcc
CXX = g++
FLAGS = -Wall -g -I./ -L./ -std=c++11 -dD -D__BUILDTIMESTAMP__=$(BUILDTIMESTAMP)
LIBS = -lWiPryClarity -lusb-1.0 -lpthread

$(EXEC): $(OBJECTS)
	$(CXX) $(FLAGS) -o $(EXEC) $(OBJECTS) $(LIBS)

.c.o:
	$(CC) -c $(FLAGS) $<

.cpp.o:
	$(CXX) -c $(FLAGS) $<

clean:
	rm -f *.o
	rm -f $(EXEC)

