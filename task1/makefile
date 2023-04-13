CC = gcc
CFLAGS = -Wall
LDFLAGS =
EXECUTABLE = myshell
OBJ_FILES = myshell.o history.o variable.o

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

myshell.o: myshell.c history.h
	$(CC) $(CFLAGS) -c $<

history.o: history.c history.h
	$(CC) $(CFLAGS) -c $<

variable.o: variable.c variable.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ_FILES) $(EXECUTABLE)
