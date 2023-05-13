CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -pedantic
LDFLAGS = -L. -lCodec

EXEC = program
MAIN = main.cpp
THREADPOOL = thread_pool.cpp
DATACHUNK = data_chunk.cpp
THREADPOOL_H = thread_pool.h
DATACHUNK_H = data_chunk.h

OBJS = $(THREADPOOL:.cpp=.o) $(DATACHUNK:.cpp=.o) $(MAIN:.cpp=.o) 

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXEC) 

$(MAIN:.cpp=.o): $(MAIN) $(THREADPOOL_H)
	$(CC) $(CFLAGS) -c $(MAIN) -o $(MAIN:.cpp=.o) -I.

$(THREADPOOL:.cpp=.o): $(THREADPOOL) $(THREADPOOL_H)
	$(CC) $(CFLAGS) -c $(THREADPOOL) -o $(THREADPOOL:.cpp=.o) -I.

$(DATACHUNK:.cpp=.o): $(DATACHUNK) $(DATACHUNK_H)
	$(CC) $(CFLAGS) -c $(DATACHUNK) -o $(DATACHUNK:.cpp=.o) -I.

clean:
	rm -f $(OBJS) $(EXEC)
