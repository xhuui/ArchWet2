CC = g++
CFLAGS = -Wall
CCLINK = $(CC)
OBJS = cache.o cacheSim.o

cacheSim: $(OBJS)
	$(CCLINK) $(CFLAGS) $(OBJS) -o cacheSim

cacheSim.o: cacheSim.cpp
cache.o: cache.cpp cache.hpp
cacheManager: cacheManager.cpp

.PHONY: clean

clean:
	rm -f *.o
	rm -f cacheSim
