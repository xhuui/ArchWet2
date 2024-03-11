CC = g++
CFLAGS = -Wall -std=c++11 -pedantic-errors -g
CCLINK = $(CC)
OBJS = cacheManager.o cacheSim.o

cacheSim: $(OBJS)
	$(CCLINK) $(CFLAGS) $(OBJS) -o cacheSim

cacheSim.o: cacheSim.cpp
cacheManager.o: cacheManager.cpp cacheManager.hpp

.PHONY: clean

clean:
	rm -f *.o
	rm -f cacheSim
